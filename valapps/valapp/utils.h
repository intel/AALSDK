/// @file utils.h
/// @brief utility functions and classes

#pragma once
#include <string>
#include <vector>
#include <typeinfo>
#include <random>

/// @brief utility functions grouped under the utils namespace
namespace utils
{
    /// @brief Use RTTI to get a type name
    /// @param[in] tinfo the typeid of a type
    /// @returns the string type name (includes namespace)
    std::string get_typename(const std::type_info &tinfo);

    /// @brief Template version of get_typename function
    /// @tparam T the type to get the type of
    /// @returns the string type name of T
    template<typename T>
    std::string get_typename()
    {
        return get_typename(typeid(T));
    }

    /// @brief Checks if a path exists in the filesystem
    //
    /// @param[in] path to a filesystem object
    /// @returns true if the object exists, false otherwise
    bool path_exists(const std::string &path);

    /// @brief Splits an input string using a string delimeter
    //
    /// @param[in] inp input string to split
    /// @param[in] delim delimeter string to split the string on
    /// @returns a vector of substrings of the input string
    // separated by the delimeter
    template<typename T>
    std::vector<T> split(const T &inp, const T &delim)
    {
        std::vector<T> out;
        std::size_t pos = 0;
        while(pos != T::npos)
        {
            auto next = inp.find(delim, pos);
            if (next == T::npos)
            {
                out.push_back(inp.substr(pos, inp.size() - pos));
                break;
            }
            else if (next > pos)
            {
                out.push_back(inp.substr(pos, next-pos));
                pos = next + delim.size();
            }
            else if (next == pos)
            {
                pos++;
            }
        }

        return out;
    }

    /// @brief random integer generator
    /// @details
    /// Used to generate random integers between
    /// a given range.
    /// Usage random<1,100> r; int number = r();
    /// @tparam lo_ the low end of the range
    /// @tparam hi_ the high end of the range
    /// @tparam IntTYpe the type of integer to generage
    /// short, int, long, unsigned, ...
    template<int lo_, int hi_, typename IntType = int>
    class random
    {
        public:
            /// @brief random<> constructor
            random():
                gen_(rdev_()),
                dist_(lo_, hi_)
            {
            }

            /// @brief generates a random integer
            /// @returns a random number between range
            /// lo_ and hi_
            int operator()()
            {
                return dist_(gen_);
            }

        private:
            std::random_device rdev_;
            std::mt19937 gen_;
            std::uniform_int_distribution<IntType> dist_;
    };

}
