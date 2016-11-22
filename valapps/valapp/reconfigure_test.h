#pragma once
#include "afu_test.h"

class reconfigure_test : public afu_test
{
    public:

        virtual void register_tests();
        virtual void setup();
        virtual void teardown();

    private:
        void sw_pr_01a(const arguments& args);
        void sw_pr_02(const arguments& args);
        void sw_pr_03(const arguments& args);
        void sw_pr_04a(const arguments& args);
        void sw_pr_05a(const arguments& args);
        void sw_pr_06a(const arguments& args);
        void sw_pr_07a(const arguments& args);
        void sw_pr_08(const arguments& args);
        void sw_pr_09(const arguments& args);
        void sw_pr_10(const arguments& args);
        void sw_pr_11(const arguments& args);
        void sw_pr_12(const arguments& args);

};
