#pragma once
#include <memory>
#include <string>
#include <vector>
#include "utils.h"
#include "Loggable.h"

class test_args : public Loggable
{
    public:
        typedef std::shared_ptr<test_args> ptr_t;

        test_args();
        test_args(void *args);
        ~test_args();

        void parse_commandline(int argc, char* argv[]);
        void add(const std::string &key, const std::string& value);

        
        bool has(const std::string &key) const;

        //template<typename T>
        //T get(const std::string &key, const T &value)
        //{
        //    return const_cast
        
        template<typename T>
        T get(const std::string & key, const T&value) const
        {
            if (has(key))
            {
                return get<T>(key);
            }
            else
            {
                return value;
            }

        }

        template<typename T>
        T get(const std::string &key) const
        {
            auto type_name = utils::get_typename<T>();
            void* raw = void_value(type_name, key);
            if (0 != raw)
            {
                T casted = *(static_cast<T*>(raw));
                delete static_cast<T*>(raw);
                return casted;
            }
            return T();
        }

        void* void_value(const std::string & type_name, const std::string &key) const;
        
        std::vector<std::string>& ordered()
        {
            return ordered_args_;
        }

        const std::vector<std::string>& ordered() const
        {
            return ordered_args_;
        }


    private:
        std::vector<std::string> ordered_args_;
        void *args_;

};
