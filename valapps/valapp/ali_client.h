#pragma once
#include "service_manager.h"
#include <aalsdk/AALTypes.h>

class ali_client
{
    public:
        ali_client()
        {
            auto client1 = service_manager::instance()->get("ALI");
            auto client2 = service_manager::instance()->get("PR");
            mmio_ = client1->get_interface<AAL::IALIMMIO>();
            umsg_ = client1->get_interface<AAL::IALIUMsg>();
            buffer_ = client1->get_interface<AAL::IALIBuffer>();
            perf_ = client1->get_interface<AAL::IALIPerf>();
            reset_ = client1->get_interface<AAL::IALIReset>();
            reconfigure_ = client2->get_interface<AAL::IALIReconfigure>();
            signaltap_ = client1->get_interface<AAL::IALISignalTap>();
        }
        virtual ~ali_client(){}

        AAL::IALIMMIO* mmio_if() { return mmio_; }
        AAL::IALIUMsg* umsg_if() { return umsg_; }
        AAL::IALIBuffer* buffer_if() { return buffer_; }
        AAL::IALIPerf* perf_if() { return perf_; }
        AAL::IALIReset* reset_if() { return reset_; }
        AAL::IALIReconfigure* reconfigure_if() { return reconfigure_; }
        AAL::IALISignalTap* signaltap_if() { return signaltap_; }
    private:
        AAL::IALIMMIO* mmio_;
        AAL::IALIUMsg* umsg_;
        AAL::IALIBuffer* buffer_;
        AAL::IALIPerf* perf_;
        AAL::IALIReset* reset_;
        AAL::IALIReconfigure* reconfigure_;
        AAL::IALISignalTap* signaltap_;
};
