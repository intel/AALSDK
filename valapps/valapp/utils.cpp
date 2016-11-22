#include "utils.h"
#include <sys/stat.h>
#ifdef __linux
#include <cxxabi.h>
#endif

namespace utils
{

    std::string get_typename(const std::type_info& tinfo)
    {
#ifdef __linux
        int status;
        std::string demangled_name = abi::__cxa_demangle(tinfo.name(), 0, 0, &status);
        return demangled_name;
#endif
        return tinfo.name();
    }

    bool path_exists(const std::string &path)
    {
        struct stat buffer;
        return 0 == stat(path.c_str(), &buffer);
    }
}
