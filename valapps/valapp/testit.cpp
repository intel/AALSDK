#include "arguments.h"
#include <iostream>
#include <string>
#include "utils.h"
#include "process.h"

int main(int argc, char* argv[])
{
    for (int i = 0; i < argc; ++i)
    {
        std::cout << argv[i] << std::endl;
    }
    auto p = utils::process::start("fpgadiag", { "--mode=lpbk1" });
    auto code = p.wait();
    std::cout << "exit code: " << code << std::endl;
    return 0;
    auto split = utils::split<std::string>("/abc/def/xyz", ":");
    for (auto s : split)
    {
        std::cout << s << std::endl;
    }
    return 0;
    arguments argparse;
    argparse("argument1", 'a', true)
            ("argument2", 'b', true)
            ("argument3", 'c', false).end();

    argparse.parse(argc, argv);

    std::cout << "argument1: " << argparse.get_string("argument1") << std::endl;
    std::cout << "argument2: " << argparse.get_string("argument2") << std::endl;
    std::cout << "argument3: " << argparse.have("argument3") << std::endl;
    std::cout << "leftover: ";
    for (auto l : argparse.leftover())
    {
        std::cout << l << ", ";
    }
    std::cout << std::endl;

}


