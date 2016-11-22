#include "test_args.h"
#include <map>
#include <functional>
#include <string>
#include <string.h>
#include <regex>
#include <json/json.h>

using namespace std;
using namespace utils;

namespace 
{
    typedef map<std::string, function<void*(const Json::Value*)>> converter_map_t;

    converter_map_t _initialize_converters()
    {
        converter_map_t converters;
        converters[get_typename<string>()] = [](const Json::Value *v){ return new string(v->asString());};
        converters[get_typename<int>()] = [](const Json::Value *v){ return new int(v->asInt());};
        converters[get_typename<long>()] = [](const Json::Value *v){ return new long(v->asInt64());};
        converters[get_typename<unsigned int>()] = [](const Json::Value *v){ return new unsigned int(v->asUInt());};
        converters[get_typename<unsigned long>()] = [](const Json::Value *v){ return new unsigned long(v->asUInt64());};
        converters[get_typename<float>()] = [](const Json::Value *v){ return new int(v->asFloat());};
        converters[get_typename<double>()] = [](const Json::Value *v){ return new long(v->asDouble());};
        converters[get_typename<bool>()] = [](const Json::Value *v){ return new long(v->asBool());};
        return converters;
    }

    static converter_map_t _converters = _initialize_converters();
    
    void* convert_to(const Json::Value *value, const std::string &type_name)
    {
        auto func_it = _converters.find(type_name);
        if (func_it == _converters.end())
        {
            return 0;
        }
        auto func = _converters[type_name];
        return func(value);
    }
}

test_args::test_args()
{
    args_ = static_cast<void*>(new Json::Value());
}

test_args::test_args(void *args)
{
    args_ = static_cast<void*>(new Json::Value(*static_cast<Json::Value*>(args)));
}

test_args::~test_args()
{
    delete static_cast<Json::Value*>(args_);
}

void test_args::parse_commandline(int argc, char* argv[])
{
    bool found_arg = false;
    std::string key, value;
    int i = 1;
    while(i < argc)
    {
        if (0 == strncmp(argv[i], "--", 2))
        {
            if (i < argc-1)
            {
                if ( 0 != strncmp(argv[i+1], "--", 2))
                {
                    add(&argv[i][2], argv[i+1]);
                    i += 2;
                }
                else
                {
                    add(&argv[i][2], "true");
                    i += 1;
                }
            }
            else
            {
                add(&argv[i][2], "true");
            }
        }
        else
        {   
            ordered_args_.push_back(argv[i]);
            i += 1;
        }
    }
}

void test_args::add(const std::string &key, const std::string &value_str)
{
    Json::Value &args = *static_cast<Json::Value*>(args_);
    if (value_str.empty())
    {
        args[key] = true;
        return;
    }
    auto is_integer = [](const string &s) { return std::all_of(s.begin(), s.end(), ::isdigit); };
    auto is_double = [is_integer](const string &s) 
    { 
        auto pos = s.find(".");
        if (pos == string::npos)
        {
            return false;
        }
        
        if (pos == 0)
        {
            return is_integer(s.substr(1));
        }
        
        return is_integer(s.substr(0, pos)) && is_integer(s.substr(pos));
    };
    
    if (is_integer(value_str))
    {
        args[key] = Json::Value::Int64(stol(value_str));
    }
    else if (is_double(value_str))
    {
        args[key] = Json::Value(stod(value_str));
    }
    else if (value_str == "true")
    {
        args[key] = Json::Value(true);
    }
    else if (value_str == "false")
    {
        args[key] = Json::Value(false);
    }
    else
    {
        args[key] = value_str;
    }
}

bool test_args::has(const std::string &key) const
{
    Json::Value *value = static_cast<Json::Value*>(args_);
    return value->isMember(key);
}

void* test_args::void_value(const std::string & type_name, const std::string &key) const
{
    Json::Value *value = static_cast<Json::Value*>(args_);
    if (value->isMember(key))
    {
        auto func_it = _converters.find(type_name);
        if (func_it == _converters.end())
        {
            Log() << "no converter found for type name " << type_name << std::endl;
            return 0;
        }
        auto func = func_it->second;
        return func(&(*value)[key]);
    }
    Log() << "no key with name " << key << " found in test_args" << std::endl;
    return 0;
}


