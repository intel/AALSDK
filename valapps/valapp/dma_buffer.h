#pragma once
#include "afu_test.h"
#include "afu_client.h"

class dma_buffer : public afu_test
{
    public:

        virtual void register_tests();
        virtual void setup();
        virtual void teardown();

    private:
        void allocate_hold(const arguments &args);
        afu_client::ptr_t afu_;
};
