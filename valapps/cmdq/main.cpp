#include "service_manager.h"
#include "nlb_client.h"
#include "reconfigure_client.h"
#include "arguments.h"


int main(int argc, char* argv[])
{
    arguments argparse;
    argparse("input",       'i', optional_argument);
    argparse("allocations", 'a', optional_argument);
    argparse("mode",        'm', optional_argument);
    argparse("cachelines",  'l', optional_argument);
    argparse("services",    's', optional_argument);
    argparse("bitstream",   'b', optional_argument);
    argparse("config",      'C', optional_argument);
    argparse("ase",         'A', no_argument);
    argparse("perfc",       'P', no_argument);
    argparse("stats",       'S', optional_argument);

    if (!argparse.parse(argc, argv))
    {
        return -1;
    }

    auto services = argparse.get_string("services", "services.json");
    auto allocations = argparse.get_int("allocations", 1);
    auto cachelines = argparse.get_int("cachelines", 64);
    auto test_mode = argparse.get_string("mode", "loopback");
    auto stats = argparse.get_string("stats", "stdout");

    auto env = argparse.have("ase") ? service_manager::hwenv_t::ase : service_manager::hwenv_t::hw;

    auto sm = service_manager::instance(env);
    sm->define_services(services);
    if (argparse.have("bitstream"))
    {
        auto afu = sm->get_service("PR", true);
        auto pr = std::dynamic_pointer_cast<reconfigure_client>(afu);
        pr->reconfigure(argparse.get_string("bitstream"), 1000, reconfigure_client::action_t::honor_request);
    }
    
    auto afu_name = test_mode == "loopback" ? "NLB0" : "NLB3";

    auto afu = sm->get_service(afu_name, true);
    auto nlb = std::dynamic_pointer_cast<nlb_client>(afu);
    nlb->status();

    nlb->configure(test_mode, argparse.get_string("config"));

    if (argparse.have("input"))
    {
        auto input = argparse.get_string("input");
        nlb->command_queue(input);
    }
    else
    {
        nlb->command_queue(cachelines, allocations);
    }
    nlb->calc_bw();

    if (stats == "stdout")
    {
        nlb->write_stats(std::cout);
        std::cout << "========================================" << std::endl;
        std::cout << "===============SUMMARY==================" << std::endl;
        std::cout << "========================================" << std::endl;
        nlb->write_summary(std::cout);
    }
    else
    {
        std::ofstream fout;
        fout.open(stats + ".csv");
        nlb->write_stats(fout);
        fout.close();
        fout.open(stats + "-summary.csv");
        nlb->write_summary(fout);
        fout.close();
    }

    sm->shutdown();
}
