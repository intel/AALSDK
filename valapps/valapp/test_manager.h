#pragma once
#include <memory>
#include <map>
#include <functional>

#include "Loggable.h"
#include "utils.h"
#include "test_context.h"
#include "afu_test.h"
#include "arguments.h"



namespace Json
{
    class Value;
}

class test_manager : public Loggable
{
    public:
        typedef std::shared_ptr<test_manager> ptr_t;
        using test_factory = std::function<afu_test*(test_context::ptr_t)>;
        //typedef afu_test* (* test_factory)(test_context::ptr_t);

        ~test_manager();

        static ptr_t instance();

        void register_arguments(int argc, char** argv);
        
        void register_arguments(const std::vector<char*> &args);
        
        inline arguments & args()
        {
            return args_;
        }

        bool register_test(test_factory fn, const std::string name)
        {
            Log() << "registering test suite: " << name << std::endl;
            test_factory_map_[name] = fn;
            return true;
        }

        template<class T>
        static bool register_test()
        {
            auto type = utils::get_typename<T>();
            test_factory fn = [](test_context::ptr_t context)
            { 
                    afu_test *t = new T();
                    t->set_context(context);
                    t->register_tests();
                    return t;
            };
            return test_manager::instance()->register_test(fn, type);
        }
        
        void shutdown();
        bool load_testlib(const std::string &libname);
        void run_tests(const std::string &test_spec);
        afu_test::results_t run_test(const std::string &suite_name, const std::string & test_name,int argc, char* argv[]); 


    private:
        test_manager();
        arguments args_;
        std::vector<std::string> raw_args_;
        void _run_suite(Json::Value *value);
        static ptr_t instance_;
        std::map<std::string, test_factory> test_factory_map_;
        std::map<std::string, void*> lib_map_;

};

