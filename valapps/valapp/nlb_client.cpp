#include "nlb_client.h"
#include "afu_test.h"
#include "utils.h"
#include <chrono>
#include <functional>
#include "client_factory.h"
#include "json/json.h"
#include <aalsdk/service/IALIAFU.h>

using namespace std::chrono;
using namespace std::placeholders;
using namespace AAL;

using usec = microseconds;

namespace
{
    static bool b = client_factory::register_client<nlb_client>();
}

static std::map<std::string, uint32_t> read_ch_names =
{
    { "va",   0x0000 },
    { "vl0",  0x1000 },
    { "vh0",  0x1000 },
    { "vh1",  0x3000 },
    { "vr",   0x4000 }
};

static std::map<std::string, uint32_t> write_ch_names =
{
    { "va",  0x0000 },
    { "vl0", 0x2000 },
    { "vh0", 0x4000 },
    { "vh1", 0x6000 },
    { "vr",  0x8000 }
};

nlb_client::nlb_client()
: cfg_(0)
, timeout_(0.0)
, continuous_(false)
, frequency_(400)
, perf_(0)
, cancel_(false)
{

}

nlb_client::~nlb_client()
{
    
}

void nlb_client::enable_perf()
{
    auto sclient = service_manager::instance()->get_service("FME", true);
    auto fme = std::dynamic_pointer_cast<afu_client>(sclient);
    if (fme->status() != service_client::status_t::allocated)
    {
        Log() << "Could not get FME" << std::endl;
    }
    else
    {
        perf_ = fme->get_interface<IALIPerf>();
    }
}

void nlb_client::release()
{
    dsm_.reset();
    inp_.reset();
    out_.reset();
    afu_client::release();
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

void nlb_client::copy_command(const cmdq_entry_t & entry)
{
    auto inp = std::get<0>(entry);
    auto out = std::get<1>(entry);
    auto lines = std::get<2>(entry);
    // set input workspace address
    mmio_write64(static_cast<uint32_t>(nlb_client::csr::src_addr), inp);
    // set output workspace address
    mmio_write64(static_cast<uint32_t>(nlb_client::csr::dst_addr), out);
    // set the number of cache lines
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::num_lines), lines);

    // set swvalid bit to 1 
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::cmdq_sw), 1);
}

void nlb_client::add_option(uint32_t opt)
{
    cfg_ |= opt;
}

void nlb_client::add_option(test_mode_t opt)
{
    add_option(static_cast<uint32_t>(opt));
}

void nlb_client::add_option(const std::string &opt)
{
        const std::string& value = opt;
        int radix = value.find("0X") == 0 || value.find("0x") == 0 ? 16 : 10;
        uint32_t num = 0;
        try
        {
            num = std::stoi(value.c_str(), nullptr, radix);
        }
        catch(...)
        {
            Log() << "error converting option (" << opt << ") to integer" << std::endl;
            return;
        }
        add_option(num);
}

void nlb_client::configure(const std::string &test_mode, const std::string &config)
{
    if (test_mode == "read")
    {
        cfg_ = static_cast<uint32_t>(test_mode_t::read);
    }
    else if (test_mode == "write")
    {
        cfg_ = static_cast<uint32_t>(test_mode_t::write);
    }
    else if (test_mode == "throughput" || 
             test_mode == "thruput" || 
             test_mode == "thput" || 
             test_mode == "tput")
    {
        cfg_ = static_cast<uint32_t>(test_mode_t::throughput);
    }
    Json::Features::all();
    Json::Value root;
    Json::Reader reader;
    std::ifstream stream(config);
    if (config != "" && utils::path_exists(config) && reader.parse(stream, root))
    {
        if (root.isMember("continuous") && root["continuous"].asBool())
        {
            //add_option(test_mode_t::cont);
            continuous_ = true;
            timeout_ = 10.0;
        }
        if (root.isMember("timeout"))
        {
            timeout_ = root["timeout"].asDouble();
        }
        if (root.isMember("frequency"))
        {
            frequency_ = root["frequency"].asUInt();
        }
        else
        {
            frequency_ = 400;
        }

        if (root.isMember("channels"))
        {
            auto ch = root["channels"];
            if (ch.isMember("read"))
            {
                auto it = read_ch_names.find(ch["read"].asString());
                if (it != read_ch_names.end())
                {
                    add_option(it->second);
                }
                else
                {
                    add_option(ch["read"].asString());
                }
            }

            if (ch.isMember("write"))
            {
                auto it = write_ch_names.find(ch["write"].asString());
                if (it != write_ch_names.end())
                {
                    add_option(it->second);
                }
                else
                {
                    add_option(ch["write"].asString());
                }

            }
        }
    }
}


void nlb_client::command_queue(uint32_t cache_lines, uint32_t allocations)
{ 
    cmdq_t fifo;
    uint32_t buffer_size = CL(1)*cache_lines;
    auto test_mode = cfg_ & 0xE;

    // allocate N number of fifo entries
    for (int i = 0; i < allocations; ++i)
    {
        auto inp = allocate_buffer(buffer_size, true);
        auto out = allocate_buffer(buffer_size, true);
        switch(test_mode)
        {
            case 0:
                ::memset(inp->address(), 0xAF, buffer_size);
                ::memset(out->address(), 0xBE, buffer_size);
                break;
            case 3:
                ::memset(inp->address(), 0x0, buffer_size);
                ::memset(out->address(), 0x0, buffer_size);
            default:
                break;
        }
        fifo.push_back(std::make_tuple(CACHELINE_ALIGNED_ADDR(inp->physical()), 
                                       CACHELINE_ALIGNED_ADDR(out->physical()), 
                                       cache_lines, 
                                       inp->address(),
                                       out->address()));
    }
    do_commands(fifo);
}

void nlb_client::command_queue(const std::string &input)
{
    cmdq_t fifo;
    std::ifstream input_stream(input);
    std::string line;
    if (input_stream.is_open())
    {
        while(input_stream.good())
        {
            getline(input_stream, line);
            auto splits = utils::split<std::string>(line, ",");
            if (splits.size() == 3)
            {
                auto inp =   std::stoi(splits[0].c_str(), nullptr, 16);
                auto out =   std::stoi(splits[1].c_str(), nullptr, 16);
                auto lines = std::stoi(splits[2].c_str(), nullptr, 10);
                fifo.push_back(std::make_tuple(inp, out, lines, nullptr, nullptr));
            }
        }
    }
    do_commands(fifo);
}

uint32_t nlb_client::swvalid_loop(cmdq_t &fifo1, cmdq_t &fifo2)
{
    uint32_t allocations = 0;
    // issue command from the command queue in the main thread
    while (!fifo1.empty())
    {
        if (continuous_ && cancel_)
        {
            break;
        }

        cmdq_entry_t &data = fifo1.front();
        copy_command(data);
        allocations++;
        fifo2.push_back(data);
        // wait for NLB to saw it's ready for next command (SWVALID is set to 0)
        wait_for_register(static_cast<uint32_t>(nlb_client::csr::cmdq_sw), 1, 0);
        fifo1.pop_front();
        if (continuous_ )
        {
            // wait for fifo1 to get entries from fifo2
            while(!cancel_ && !fifo2.empty() && fifo1.empty())
            {
                std::this_thread::sleep_for(usec(10));
            }
        }
    }
    return allocations;
}

uint32_t nlb_client::hwvalid_loop(cmdq_t &fifo1, cmdq_t &fifo2)
{
    // use fifo2 as input

    wait_for_register(static_cast<uint32_t>(nlb_client::csr::cmdq_hw), 1, 1);
    uint32_t iteration = 0, dsm_number =0;
    while(!cancel_)
    {
        while(!fifo2.empty())
        {
            if (continuous_ && cancel_)
            { 
                std::cout << "dsm_number -> " << dsm_number << std::endl;
                return iteration;
            }
            // DSM status space will alternate on even/odd iterations between
            // the DSM base address + 0x40 and DSM base address + DSM length + test_complete offset
            // Adjust according to iteration number (even vs odd)
            btPhysAddr offset = static_cast<btPhysAddr>(nlb_client::dsm::test_complete) + 
                                (iteration % 2) * static_cast<btPhysAddr>(nlb_client::dsm::test_complete);
            volatile bt32bitCSR *dsm_status_addr = (volatile bt32bitCSR*)(dsm_->address() + offset);
            // Wait for HWVALID register to be set to 0
            wait_for_register(static_cast<uint32_t>(nlb_client::csr::cmdq_hw), 1, 0);
            if (iteration == 0)
            {
                // Wait for status bit to be set to 1
                // Becasue NLB uses dsm number as zero based, 
                // for the first iteration, wait until the status is 1
                while ( 0 == ((*dsm_status_addr)&0x1) )
                {
                    if (cancel_)
                    {
                        break;
                    }
                    std::this_thread::sleep_for(usec(10));
                }
                //std::cout << "dsm_status 0x" << ((*dsm_status_addr)&0x1) << std::endl;
            }
            else
            {
                // Othersize, wait for NLB to write index to dsm number
                while ( iteration != ((*dsm_status_addr)>>0x1))
                {
                    if (continuous_ && cancel_)
                    {
                        break;
                    }
                    std::this_thread::sleep_for(usec(10));
                }
                if (!cancel_)
                {
                    dsm_number = ((*dsm_status_addr)>>1);
                }
                //if (!cancel_)
                //{
                //    std::cout << "iteration  -> " << iteration << ", ";
                //    std::cout << "offset     -> 0x" << std::hex << offset << std::dec << ", ";
                //    std::cout << "dsm_number -> " << ((*dsm_status_addr)>>1)  << std::endl;
                //}
            }

            if (!cancel_)
            { 
                bool passed = verify(fifo2.front());
                save_stat(iteration++, passed);
            }
            // if we're running in continuous mode
            if (continuous_ && !cancel_)
            {
                // recycle entries from  fifo2 into fifo1
                fifo1.push_back(fifo2.front());
            }
            fifo2.pop_front();
            
            // Wait for HWVALID register to be set back to 1
            wait_for_register(static_cast<uint32_t>(nlb_client::csr::cmdq_hw), 1, 1);
            // Reset the status bit to 0
            //*dsm_status_addr &= 0xFFFE;
        }
        std::this_thread::sleep_for(usec(10));
    }
    std::cout << "dsm_number -> " << dsm_number << std::endl;
    return iteration;
}

bool nlb_client::wait_for_done(uint32_t allocations)
{
    btPhysAddr offset = static_cast<btPhysAddr>(nlb_client::dsm::test_complete) + 
                                ((allocations-1) % 2) * static_cast<btPhysAddr>(nlb_client::dsm::test_complete);
    volatile bt32bitCSR *dsm_status_addr = (volatile bt32bitCSR*)(dsm_->address() + offset);

    // wait until NLB writes the allocation index (0 based)
    while ( allocations > ((*dsm_status_addr)>>0x1)+1)
    {
        std::this_thread::sleep_for(usec(10));
    }
    while ( true )
    {
        std::this_thread::sleep_for(usec(10));
        if (cancel_)
        {
            return false;
        }
        if (mmio_read32(static_cast<uint32_t>(nlb_client::csr::cmdq_sw)) == 0 &&
            mmio_read32(static_cast<uint32_t>(nlb_client::csr::cmdq_hw)) == 0 )
        {
            return true;
        }
    }
}

void nlb_client::do_commands(cmdq_t &fifo)
{
    cmdq_t fifo2;
    uint32_t allocations = 0;
    reset();
    
    dsm_ = allocate_buffer(dsm_size, true);
    // set dsm base, high then low
    mmio_write64(static_cast<uint32_t>(nlb_client::dsm::basel), dsm_->physical());
    // assert afu reset
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 0);
    // clear the dsm status fields
    //::memset(dsm_->address(), 0, dsm_size);
    // de-assert afu reset
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 1);
    // set the test mode
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::cfg), cfg_);
    // spawn another thread to check for HWVALID being set to 0
    // when HWVALID is 0, then check the buffers for the test mode
    // eg. test mode is 0, then check output buffers match input buffers

    std::future<uint32_t> swvalid_loop = std::async(std::launch::async, 
            &nlb_client::swvalid_loop, this, std::ref(fifo), std::ref(fifo2));

    std::future<uint32_t> hwvalid_loop = std::async(std::launch::async, 
            &nlb_client::hwvalid_loop, this, std::ref(fifo), std::ref(fifo2));
    
    if (continuous_)
    {
        std::this_thread::sleep_for(duration<double>(timeout_)); 
        cancel_ = true;
        swvalid_loop.wait();
        hwvalid_loop.wait();
        // stop the device
        mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 7);
    }
    else
    {
        swvalid_loop.wait();
        cancel_ = true;
        hwvalid_loop.wait();
    }


    allocations = swvalid_loop.get();
    auto iterations = hwvalid_loop.get();

    // if we're not running in continuous mode, 
    // let's wait for nlb to finish by calling
    // wait_for_done but we'll only wait for 
    // a given amount of time (1 sec)
    cancel_ = false; 
    std::future<bool> done_thread = std::async(std::launch::async, 
            &nlb_client::wait_for_done, this, allocations);
    const auto fs = done_thread.wait_for(seconds(1));
    if (fs != std::future_status::ready)
    {
        // cancel it if it times out
        Log() << "Error waiting for dsm status" << std::endl;
        cancel_ = true;
    }
    else
    {
        bool value = done_thread.get();
        //Log() << "Done with status result of " << value << " and status of 0x" << std::hex << *dsm_status << std::endl;
    }

    // stop the device
    mmio_write32(static_cast<uint32_t>(nlb_client::csr::ctl), 7);
}


bool nlb_client::wait_for_register(uint32_t offset, uint32_t mask, uint32_t value)
{
    auto ctl = mmio_read32(offset);
    while (!cancel_ && ctl & mask != value)
    {
        std::this_thread::sleep_for(usec(10));
        ctl = mmio_read32(offset);
    }
    if (cancel_)
    {
        return false;
    }
    
    return true;
}


void nlb_client::save_stat(uint32_t iteration, bool passed)
{
    uint32_t rdhit = 0, wrhit = 0, rdmiss = 0, wrmiss = 0;
    if (perf_)
    {
        NamedValueSet nvs;
        perf_->performanceCountersGet(&nvs);
        if (!nvs.Get(AALPERF_READ_HIT, &rdhit))
        {

        }
        if (!nvs.Get(AALPERF_WRITE_HIT, &wrhit))
        {

        }
        if (!nvs.Get(AALPERF_READ_MISS, &rdmiss))
        {

        }
        if (!nvs.Get(AALPERF_WRITE_MISS, &wrhit))
        {

        }
    }

    auto address = dsm_->address() + (iteration%2)*static_cast<uint32_t>(dsm::test_complete);
    stats_.push_back(run_stat
       {
           iteration,
           cfg_,
           passed,
           *(volatile bt32bitCSR*)(address + static_cast<uint32_t>(dsm::test_error)),
           *(volatile bt32bitCSR*)(address + static_cast<uint32_t>(dsm::num_clocks)),
           *(volatile bt32bitCSR*)(address + static_cast<uint32_t>(dsm::num_reads)),
           *(volatile bt32bitCSR*)(address + static_cast<uint32_t>(dsm::num_writes)),
           *(volatile bt32bitCSR*)(address + static_cast<uint32_t>(dsm::start_overhead)),
           *(volatile bt32bitCSR*)(address + static_cast<uint32_t>(dsm::end_overhead))
       });
}

void nlb_client::write_stats(std::ostream & stream)
{
    stream << std::dec;
    stream << "iteration, cfg, passed, dsm_errors, dsm_clocks, dsm_reads, dsm_writes, read_bw, write_bw, rdhit, wrhit, rdmiss, wrmiss" << std::endl;
    for (const run_stat &stat : stats_)
    {
        stream << stat << std::endl;
    }

}

void nlb_client::write_summary(std::ostream & stream)
{
    stream << std::dec;
    stream << "iterations, cfg, total_ticks, total_reads, total_writes, read_bw, write_bw" << std::endl;
    stream << summary_.iteration  << ", ";
    stream << summary_.test_mode  << ", ";
    stream << summary_.dsm_clocks << ", ";
    stream << summary_.dsm_reads  << ", ";
    stream << summary_.dsm_writes << ", ";
    stream << summary_.read_bw    << ", ";
    stream << summary_.write_bw   <<  std::endl;
}

bool nlb_client::verify(const cmdq_entry_t &entry)
{
    auto inp = std::get<0>(entry);
    auto out = std::get<1>(entry);
    auto lines = std::get<2>(entry);
    auto inp_addr = std::get<3>(entry);
    auto out_addr = std::get<4>(entry);
    auto buffer_size = lines*64;
    auto test_mode = cfg_ & 0xE;
    switch(test_mode)
    {
        case 0:
            if (inp_addr && out_addr)
            {
                return 0 == ::memcmp(out_addr, 
                                     inp_addr, 
                                     buffer_size);
            }
            break;
        case 3:
            break;
        case 7:
            break;
        default:
            break;
    }
    return false;
}

void nlb_client::calc_bw()
{
    double gig = 1024.*1024.*1024.;
    double freq = frequency_ * 1.0E6;
    uint32_t total_ticks = 0;
    uint32_t total_reads = 0;
    uint32_t total_writes = 0;
    uint32_t total_read_oh = 0;
    uint32_t total_write_oh = 0;
    uint32_t total_iterations = 0;
    for( run_stat & stat : stats_)
    {
        uint32_t ticks = stat.dsm_clocks - (stat.dsm_start + stat.dsm_end);
        double read_bw = (stat.dsm_reads * (CL(1) * freq))/ticks;
        stat.read_bw = read_bw/gig;
        double write_bw = (stat.dsm_writes * (CL(1) * freq))/ticks;
        stat.write_bw = write_bw/gig;
        if (ticks > 0)
        {
            total_ticks += ticks;
            total_reads += stat.dsm_reads;
            total_writes += stat.dsm_writes;
            total_iterations += 1;
        }
    }

    summary_ = run_stat
    {
        total_iterations,
        cfg_,
        false,
        0,
        total_ticks,
        total_reads,
        total_writes
    };
    summary_.read_bw = (total_reads * (CL(1) * freq))/(gig*total_ticks);
    summary_.write_bw = (total_writes * (CL(1) * freq))/(gig*total_ticks);
}
