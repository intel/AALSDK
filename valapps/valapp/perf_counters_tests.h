#pragma once
#include "afu_test.h"
#include "afu_client.h"
#include <aalsdk/AALTypes.h>

class perf_counters_tests : public afu_test
{
    public:

        virtual void register_tests();
        virtual void setup();
        virtual void teardown();

    private:
        void perfc01(const arguments& args);
        void perfc02(const arguments& args);
        void perfc03(const arguments& args);
        
        bool validate(const AAL::NamedValueSet &lhs, const AAL::NamedValueSet &rhs);

        afu_client::ptr_t nlb0_;
        afu_client::ptr_t fme_;
};
