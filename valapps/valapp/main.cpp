#include "service_manager.h"
#include "test_manager.h"
#include "test_args.h"
//#include <aalsdk/service/IALIAFU.h> 
#include <iostream>
#include <string>
#include "afu_test.h"
#include "arguments.h"

int main(int argc, char* argv[])
{
    int exit_code = 0;
    arguments argparse;
    argparse("services", 's', optional_argument, "service spec file")
            ("testspec", 'x', optional_argument, "test spec file");

    argparse.parse(argc, argv);

    std::string services_file = argparse.get_string("services", "services.json");

    
    service_manager::ptr_t manager = service_manager::instance();
    manager->define_services(services_file);
    
    auto tm = test_manager::instance();
    if (argparse.have("testspec"))
    {
        tm->run_tests(argparse.get_string("testspec"));
    }
    else
    {
        auto leftover = argparse.leftover();
        if (leftover.size() >= 3)
        {
            tm->load_testlib(leftover[0]);
            auto results = tm->run_test(leftover[1], leftover[2], leftover.size()-2, &leftover[2]); 
            for (auto result : results)
            {
                if (std::get<1>(result) != afu_test::status_pass)
                {
                    exit_code = -1;
                }
                std::cerr << "Test: " << std::get<0>(result) << " " << std::get<1>(result) << " " << std::get<2>(result) << std::endl;
            }
        }
    }
    tm->shutdown();
    manager->shutdown();
    return exit_code;
}
