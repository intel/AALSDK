#pragma once
#include "afu_test.h"
#include "afu_client.h"

class mmio_mapping : public afu_test
{
    public:

        virtual void register_tests();
        virtual void setup();
        virtual void teardown();

    private:
        void mmio_write_onecl(const arguments& args);
        void mmio_whole_region(const arguments& args);
        void mmio_bounds_check(const arguments& args);
        void mmio_length(const arguments& args);
        void dfh_feature(const arguments& args);
        void corrupt_dfh(const arguments& args);
        void mmio_stress(const arguments& args);

        afu_client::ptr_t nlb0_;

};
