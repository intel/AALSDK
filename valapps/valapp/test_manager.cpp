#include "test_manager.h"
#include "afu_test.h"
#include <dlfcn.h>
#include "json/json.h"
#include "utils.h"

using namespace std;

test_manager::ptr_t test_manager::instance_ = test_manager::ptr_t(0);

test_manager::test_manager()
{

}

test_manager::~test_manager()
{

}

test_manager::ptr_t test_manager::instance()
{
    if (0 == instance_.get())
    {
        instance_.reset(new test_manager());
    }
    return instance_;
}

void test_manager::register_arguments(int argc, char** argv)
{
    args_ = arguments(argc, argv, true);
}

void test_manager::register_arguments(const vector<char*> &args)
{
    args_ = arguments(args.size(), const_cast<char**>(args.data()));
}

void test_manager::shutdown()
{
    test_factory_map_.clear();
    for (auto it : lib_map_)
    {
        Log() << "Unloading " << it.first << std::endl;
        dlclose(it.second);
    }
}

bool test_manager::load_testlib(const std::string &libname)
{
    auto it = lib_map_.find(libname);
    if (it == lib_map_.end())
    {
        void* handle = dlopen(libname.c_str(), RTLD_LAZY);
        if (0 == handle)
        {
            Log() << "Error loading library (" << libname << "): " << dlerror() << std::endl;
        }
        else
        {
            lib_map_[libname] = handle;
            return true;
        }
    }
    return false;
}

void test_manager::run_tests(const std::string &test_spec)
{
    Json::Value root;
    Json::Reader reader;
    std::ifstream stream(test_spec);
    if (reader.parse(stream, root))
    {
        if (root.isMember("suites"))
        {
            for (auto suite : root["suites"])
            {
                _run_suite(&suite);
            }
        }
        else if (root.isMember("class"))
        {
            _run_suite(&root["class"]);
        }
    }
    else
    {
        Log() << "Could not parse test spec file (" << test_spec << "): " << reader.getFormattedErrorMessages() << std::endl;
    }
}

afu_test::results_t test_manager::run_test(const std::string & suite_name, const std::string & test_name, int argc, char* argv[]) 
{
    afu_test::results_t results;
    auto it = test_factory_map_.find(suite_name);
    if (it == test_factory_map_.end())
    {
        Log() << "No suite found with name: " << suite_name << std::endl;
        return results;
    }
    test_context::ptr_t context(0);
    test_manager::test_factory factory = it->second;
    afu_test* test = factory(context);
    auto tests = test->discover(test_name);
    if (tests.size() == 0)
    {
        Log() << "WARNING: Could not find tests for pattern " << test_name << std::endl;
        return results;
    }
    for (auto test_it : tests)
    {
        auto name = std::get<0>(test_it);
        auto fn = std::get<1>(test_it);
        auto args = std::get<2>(test_it);
        afu_test::test_status status = afu_test::test_status::status_notrun;
        std::string comment = "None";
        try
        {
            args.parse(argc, argv);
            test->setup();
            fn(args);
            status = afu_test::test_status::status_pass;
            test->teardown();
        }
        catch(test_failure &f)
        {
            status = afu_test::test_status::status_fail;
            comment = std::string(f.what());
            Log() << "Test failure: " << comment << std::endl;
        }
        catch(test_error &e)
        {
            status = afu_test::test_status::status_error;
            comment = e.what();
            Log() << "Caught exception: " << e.what() << std::endl;
        }
        catch(std::exception &e)
        {
            status = afu_test::test_status::status_error;
            comment = e.what();
            Log() << "Caught exception: " << e.what() << std::endl;
        }
        catch(...)
        {
            status = afu_test::test_status::status_error;
            comment = "unknown exception thrown by test";
            Log() << "Caught unknown exception" << std::endl;
        }
        results.push_back(std::make_tuple(name, status, comment));

    }
    delete test;
    return results;
}

void test_manager::_run_suite(Json::Value *value_ptr)
{
    Json::Value &value = *value_ptr;
    std::string testlib = value["libname"].asString();
    std::string suite = value["class"].asString();
    load_testlib(testlib);
    auto tests = value["tests"];
    for (auto test : tests)
    {
        auto name = test["test"].asString();

        if (test.get("disabled", false).asBool())
        {   
            Log() << "Test: " << name << " is disabled. Skipping" << std::endl;
        }
        else
        {
            Json::Value noargs(Json::arrayValue);
            auto args = test.get("args", noargs);
            vector<char*> values(args.size());
            if (args.isArray() && args.size() > 0)
            {
                transform(args.begin(), args.end(), values.begin(), 
                        [](Json::Value v) 
                        { 
                            return const_cast<char*>(v.asString().c_str());
                        });
            }
            else if (args.isString() && !args.empty())
            {
                auto split = utils::split<string>(args.asString(), " ");
                transform(split.begin(), split.end(), values.begin(),
                        [](const string & str)
                        {
                            return const_cast<char*>(str.c_str());
                        });
            }
            run_test(suite, name, values.size(), &values[0]);
        }
    }
    
}
