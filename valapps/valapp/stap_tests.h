#pragma once
#include "afu_test.h"

class stap_tests : public afu_test
{
    public:

        virtual void register_tests();
        virtual void setup();
        virtual void teardown();
        //virtual void run();

    private:
        void stap01(const arguments& args);
};
