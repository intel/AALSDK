#include "arguments.h"
#include <iostream>
#include <algorithm>

using namespace std;

arguments::arguments(bool suppress_errors)
    : short_options_("")
    , ended_(false)
    , suppress_errors_(suppress_errors)
{
}

arguments::arguments(int argc, char* argv[], bool suppress_errors)
    : short_options_("")
    , ended_(false)
    , suppress_errors_(suppress_errors)
{
    std::copy(&argv[0], &argv[argc], std::back_inserter(raw_));
}

arguments::~arguments()
{

}

arguments& arguments::operator()(const std::string& name,
                           char short_option,
                           int required,
                           const std::string helpstring)
{
    // save the argument - we use .c_str() in the option
    argnames_.push_back(name);
    auto saved = argnames_.back();
    short_map_[short_option] = options_.size();
    options_.push_back({ saved.c_str(),
                         required, 
                         0,
                         short_option});
    short_options_ += short_option;
    if (required == required_argument)
    {
        short_options_ += ":";
    }
    else if (required == optional_argument)
    {
        short_options_ += "::";
    }
    return *this;
}

void arguments::end()
{
    if (!ended_)
    {
        (*this)("help", 'h', no_argument, "show this help message");
        options_.push_back({0,0,0,0});
        ended_ = true;
    }
}

bool arguments::parse(bool print_help)
{
    if (raw_.size() == 0)
    {
        return false;
    }
    int argc = raw_.size();
    std::vector<char*> args(argc);
    std::transform(raw_.begin(), raw_.end(), args.begin(), 
            [](std::string &str)
            {
                return const_cast<char*>(str.c_str());
            });
    return parse(argc, &args[0], print_help);
}

bool arguments::parse(int &argc, char* argv[], bool print_help)
{
    end();
    opterr = suppress_errors_ ? 0 : opterr;
    optind = 1;
    int c;
    for (int i = 0;i < argnames_.size(); ++i)
    {
        options_[i].name = argnames_[i].c_str();
    }
    while(true)
    {
        int option_index = -1;
        c = getopt_long(argc, argv, 
                        short_options_.c_str(), 
                        options_.data(),
                        &option_index);
        if (c == -1)
        {
            break;
        }
        else if (c != '?')
        {
            option_index = option_index == -1 ? short_map_[c] : option_index;
            auto opt = options_[option_index];
            switch(opt.has_arg)
            {
                case no_argument:
                    argvalues_[opt.name] = true;
                    if (std::string(opt.name) == "help")
                    {
                        if (print_help)
                        {
                            print_usage(std::cerr);
                        };
                        return false;
                    }
                    break;
                case optional_argument:
                    argvalues_[opt.name] = optarg == 0 ? argv[optind++] : optarg;
                    break;
                case required_argument:
                    argvalues_[opt.name] = optarg == 0 ? argv[optind] : optarg;
                    break;
                default:
                    std::cerr << "Invalid value for option.has_arg" << std::endl;
                    break;
            }
        }
    }
    while(optind < argc)
    {
        leftover_.push_back(argv[optind++]);
    }
    for( auto opt : options_)
    {
        if (opt.has_arg == required_argument && 
            argvalues_.find(opt.name) == argvalues_.end())
        {
            if (print_help)
            {
                print_usage(std::cerr);
            }

            return false;
        }

    }
    argc = leftover_.size()+1;
    std::copy(leftover_.begin(), leftover_.end(), &argv[1]);
    return true;
}

void arguments::print_usage(std::ostream &stream, const std::string &additional)
{
    for ( auto opt : options_)
    {
        if (0 == opt.name)
        {
            break;
        }
        if (opt.has_arg == optional_argument)
        {
            stream << "[";
        }
        stream << "--" << opt.name << "|-" << static_cast<char>(opt.val);
        if (opt.has_arg != no_argument)
        {
            stream << " <value> "; 
        }
        if (opt.has_arg == optional_argument)
        {
            stream  << "]";
        }
        stream << " ";
        //stream << std::endl;
    }
    stream << additional << std::endl;
}

bool arguments::have(const std::string& name, bool warn)
{
    auto iter = argvalues_.find(name);
    if (iter != argvalues_.end())
    {
        return true;
    }

    if (warn)
    {
        std::cerr << "WARNING: no value found for argument " << name << std::endl;
    }

    return false;
}


bool arguments::have(const std::string& name, bool warn) const
{
    auto iter = argvalues_.find(name);
    if (iter != argvalues_.end())
    {
        return true;
    }

    if (warn)
    {
        std::cerr << "WARNING: no value found for argument " << name << std::endl;
    }

    return false;
}

std::string arguments::get_string(const std::string& name, const std::string default_value)
{
    return have(name, false) ? argvalues_[name] : default_value;
}

std::string arguments::get_string(const std::string& name, const std::string default_value) const
{
    return have(name, false) ? argvalues_.at(name) : default_value;
}

int arguments::get_int(const std::string& name, int default_value)
{
    if (have(name, false))
    {
        const std::string& value = argvalues_[name];
        int radix = value.find("0X") == 0 || value.find("0x") == 0 ? 16 : 10;
        return std::stoi(value.c_str(), nullptr, radix);
    }
    return default_value;
}

int arguments::get_int(const std::string& name, int default_value) const
{
    if (have(name, false))
    {
        const std::string& value = argvalues_.at(name);
        int radix = value.find("0X") == 0 || value.find("0x") == 0 ? 16 : 10;
        return std::stoi(value.c_str(), nullptr, radix);
    }
    return default_value;
}

long arguments::get_long(const std::string& name, long default_value)
{
    if (have(name, false))
    {
        const std::string& value = argvalues_[name];
        int radix = value.find("0X") == 0 || value.find("0x") == 0 ? 16 : 10;
        return std::stol(value.c_str(), nullptr, radix);
    }
    return default_value;
}

long arguments::get_long(const std::string& name, long default_value) const
{
    if (have(name, false))
    {
        const std::string& value = argvalues_.at(name);
        int radix = value.find("0X") == 0 || value.find("0x") == 0 ? 16 : 10;
        return std::stol(value.c_str(), nullptr, radix);
    }
    return default_value;
}

float arguments::get_float(const std::string& name, float default_value)
{
    return have(name, false) ? std::stof(argvalues_[name].c_str(), nullptr) : default_value;
}

float arguments::get_float(const std::string& name, float default_value) const
{
    return have(name, false) ? std::stof(argvalues_.at(name).c_str(), nullptr) : default_value;
}

double arguments::get_double(const std::string& name, double default_value)
{
    return have(name, false) ? std::stod(argvalues_[name].c_str(), nullptr) : default_value;
}

double arguments::get_double(const std::string& name, double default_value) const
{
    return have(name, false) ? std::stod(argvalues_.at(name).c_str(), nullptr) : default_value;
}


