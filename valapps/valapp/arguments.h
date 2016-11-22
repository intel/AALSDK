/// @file arguments.h
/// @brief arguments.h contains the arguments class
#pragma once
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <ostream>
#include <unistd.h>
#include <getopt.h>

/// @brief arguments is a wrapper around the POSIX getopt API as a 
/// helper class to specify and parse command line arguments
class arguments
{
    public:
        arguments(bool suppress_errors = false);
        arguments(int argc, char* argv[], bool suppress_errors = false);
        ~arguments();

        arguments& operator()(const std::string& name, 
                              char short_option,
                              int required = required_argument,
                              const std::string helpstring = "");
        void end();

        bool parse(bool print_help = true);

        bool parse(int &argc, char* argv[], bool print_help = true);

        void print_usage(std::ostream& stream = std::cout, const std::string &additional = "");
        
        bool have(const std::string& name, bool warn = false);
        
        bool have(const std::string& name, bool warn = false) const;
        
        std::string get_string(const std::string& name, const std::string default_value="");
        
        std::string get_string(const std::string& name, const std::string default_value="") const;

        int get_int(const std::string& name, int default_value = 0);
        
        int get_int(const std::string& name, int default_value = 0) const;
        
        long get_long(const std::string& name, long default_value = 0);
        
        long get_long(const std::string& name, long default_value = 0) const;

        float get_float(const std::string& name, float default_value = 0.0);
        
        float get_float(const std::string& name, float default_value = 0.0) const;
        
        double get_double(const std::string& name, double default_value = 0.0);
        
        double get_double(const std::string& name, double default_value = 0.0) const;

        const std::vector<char*>& leftover()
        {
            return leftover_;
        }

        const std::vector<char*>& leftover() const
        {
            return leftover_;
        }

        //bool get_bool(const std::string& name);


        friend std::ostream& operator << (std::ostream& stream, const arguments &args)
        {
            for ( std::vector<struct option>::const_iterator opt = args.options_.begin(); 
                  opt != args.options_.end();
                  ++opt)
            {
                stream << "--" << opt->name << " (" << opt->has_arg << ") ";
            }
            return stream;
        }
    private:
        std::vector<std::string> raw_;
        std::vector<std::string> argnames_;
        std::vector<char*> leftover_;
        std::map<std::string, std::string> argvalues_;
        std::map<char, int> short_map_;
        std::vector<struct option> options_;
        std::string short_options_;
        bool ended_;
        bool suppress_errors_;
};
