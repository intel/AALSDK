#pragma once
#include <csignal>
#include <atomic>
#include <functional>
#include <initializer_list>
#include <exception>

#include "test_context.h"
#include "test_args.h"
#include "arguments.h"
#include "Loggable.h"
#include "afu_client.h"

class test_error : public std::exception
{
    public:
        test_error(const char* file, const char* func, int line, const char* what) 
            : std::exception() 
            , what_(what)
            , file_(file)
            , func_(func)
            , line_(line)
            {
                message_ = std::string("message: " + what_ + ", " +
                                       "file: " + file_ + ", " +
                                       "function: " + func_ + ", " +
                                       "line: " + std::to_string(line_));

            }
        virtual ~test_error(){}

        template<typename... Types>
        static std::string message(Types... args)
        {

                std::stringstream ss;
                int dummy[sizeof...(Types)] = { (ss << args, 0)... };
                return ss.str();
        }

        virtual const char* what() const noexcept
        {
            return message_.c_str();
        }

    private:
        std::string what_;
        std::string file_;
        std::string func_;
        int line_;
        std::string message_;
};

class test_failure : public test_error
{
    public:
        test_failure(const char* file, const char* func, int line, const char* what) 
            : test_error(file, func, line, what){}
        ~test_failure(){}

        virtual const char* what() const noexcept
        {
            return test_error::what();
        }
};

namespace
{
    volatile static std::atomic<int> global_signal;
    volatile static std::atomic<bool> global_signal_raised(false);
}

extern "C"
{
    static void signal_handler(int signal)
    {
        global_signal = signal;
        global_signal_raised = true;
    }
}

#define TEST_ERROR(c, ...) if (c) throw test_error(__FILE__, __func__, __LINE__, test_error::message(__VA_ARGS__).c_str());
#define THROW_ERROR(...) throw test_error(__FILE__, __func__, __LINE__, test_error::message(__VA_ARGS__).c_str());
#define TEST_FAIL(c, ...) if (c) throw test_failure(__FILE__, __func__, __LINE__, test_error::message(__VA_ARGS__).c_str());
#define EXPECT_SIGNAL(expected_signal, ...) \
    std::signal(expected_signal, signal_handler);\
    __VA_ARGS__;\
    std::signal(expected_signal, SIG_DFL);\
    {\
        int raised_signal = global_signal.exchange(0);\
        bool raised = global_signal_raised.exchange(false);\
        if (raised && raised_signal != expected_signal || !raised)\
        {\
            throw test_failure(__FILE__, __func__, __LINE__, test_error::message("Expected signal not raised: ", expected_signal).c_str());\
        }\
    }   

class afu_test : public Loggable
{
    public:
        struct constants
        {
            static const std::size_t KB = 1024;
            static const std::size_t MB = 1024*1024;
            static const std::size_t GB = 1024*1024*2024;
        };
        enum test_status
        {
            status_pass = 0,
            status_fail,
            status_error,
            status_notfound,
            status_skipped,
            status_notrun
        };
        using test_function = std::function<void(const arguments&)>;
        typedef std::pair<test_function, arguments> fn_args_pair_t;
        typedef std::vector<std::tuple<std::string, test_function, arguments>> meta_runnable_t;
        typedef std::tuple<std::string, test_status, std::string> result_t;
        typedef std::vector<result_t> results_t;

        virtual ~afu_test(){}
        virtual void register_tests() = 0;
        virtual void setup(){}
        virtual void teardown(){}

        virtual void set_context(test_context::ptr_t context)
        {
            context_ = context;
        }

        virtual std::vector<std::string>& list() final
        {
            return test_list_;
        }

        virtual const std::vector<std::string>& list() const final
        {
            return test_list_;
        }

        virtual meta_runnable_t discover(const std::string &pattern = "*")
        {
            meta_runnable_t tests;
            auto match = [](const std::string &p, const std::string &s)
            {
                if (p == "*")
                {
                    return true;
                }
                if (p == s)
                {
                    return true;
                }
                return false;
            };
            for (auto fn_args_it : test_map_)
            {
                if (match(pattern, fn_args_it.first))
                {
                    tests.push_back(std::make_tuple(fn_args_it.first, 
                                                    fn_args_it.second.first, 
                                                    fn_args_it.second.second));
                }
            }

            return tests;
        }

    protected:
        virtual test_context::ptr_t context() final
        {
            return context_;
        }

        virtual afu_client::ptr_t get_afu_client(const std::string &name) final
        {
            auto afu = context()->get_service<afu_client>(name);
            TEST_FAIL(!afu, "Could not cast service_client to afu_client");
            TEST_FAIL(service_client::allocated != afu->status(), name,
                      " has not been allocated.");
            return afu;
        }

        template<class T>
        arguments& register_test(const std::string &name, void(T::* fn)(const arguments&))//test_function fn) final
        {
            test_list_.push_back(name);
            test_map_[name] = std::make_pair(std::bind(fn, static_cast<T*>(this), std::placeholders::_1), arguments());
            return test_map_[name].second;
        }

    private:
        test_context::ptr_t context_;
        std::map<std::string, fn_args_pair_t> test_map_;
        std::vector<std::string> test_list_;
};

