#pragma once
#include <aalsdk/AALTypes.h>
#include <aalsdk/service/IALIAFU.h>
#include <thread>
#include <future>
#include <map>
#include "service_client.h"
#include "afu_register.h"
#include "mmio_buffer.h"

class buffer_allocate_error : public std::exception
{

};

class afu_client : public service_client
{
    public:
        typedef std::shared_ptr<afu_client> ptr_t;
        enum status_t
        {
            status_none,
        };

        afu_client();

        virtual ~afu_client();

        virtual void release();

        void add_register(const afu_register &reg)
        {
            registers_.emplace(reg.id(), reg);
        }

        int reset();

        unsigned char* mmio_address();

        std::size_t mmio_length();

        void mmio_write32(unsigned int offset, int value);

        void mmio_write64(unsigned int offset, long value);

        unsigned int mmio_read32(unsigned int offset);

        long long unsigned int mmio_read64(unsigned int offset);

        bool feature_id_offset(uint32_t feature_id, uint32_t &offset);

        bool feature_type_offset(uint32_t feature_type, uint32_t &type);

        mmio_buffer::ptr_t allocate_buffer(std::size_t size, bool track_buffer = false);

        void release_buffer(AAL::btVirtAddr address);

        bool register_offset(const std::string &regid, unsigned int &offset);

        bool register_write32(const std::string &regid, int value);
        bool register_write64(const std::string &regid, long value);
        unsigned int register_read32(const std::string &regid);
        long long unsigned int register_read64(const std::string &regid);

        // <IServiceClient interface>
        void serviceAllocated(AAL::IBase *pServiceBase, AAL::TransactionID const &rTransID);
        // </IServiceClient interface>

    private:
        std::mutex mutex_;
        std::condition_variable cv_;
        std::map<AAL::btID, status_t> transaction_map_;
        std::map<std::string, afu_register> registers_;
        std::map<AAL::btVirtAddr, mmio_buffer::ptr_t> buffer_map_;
        AAL::IALIMMIO *mmio_;
        AAL::IALIUMsg *umsg_;
        AAL::IALIBuffer *buffer_;
        AAL::IALIPerf *perf_;
        AAL::IALIReset *reset_;
        AAL::IALISignalTap *stap_;


};
