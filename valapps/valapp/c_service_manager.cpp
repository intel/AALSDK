#include "c_service_manager.h"
#include <aalsdk/AALTypes.h>
#include "service_manager.h"
#include "service_client.h"
#include "afu_client.h"
#include "reconfigure_client.h"


using namespace AAL;
using namespace std;


void service_manager_shutdown()
{
    service_manager::instance()->shutdown(); 
}

void service_manager_start()
{
    service_manager::instance()->start();
}

void service_manager_define_services(char* config_file)
{
    service_manager::instance()->define_services(config_file);
}

void service_manager_create_service(char* service_name)
{
    auto service = service_manager::instance()->create_service(service_name);
    auto status = service->status();
}

void service_manager_create_services(char* config_file)
{
    auto mgr = service_manager::instance();
    mgr->start();
    mgr->create_services(config_file);
}

void* service_manager_get_client(char* name)
{
    auto client = service_manager::instance()->get_service(name);
    return client.get();
}

void service_release(char* name)
{
    service_manager::instance()->release_service(name);
}

unsigned int reconfigure(char* filename)
{
    auto reconf = dynamic_pointer_cast<reconfigure_client>(service_manager::instance()->get_service("PR", true));
    reconf->status();
    auto status = reconf->reconfigure(filename, 1000, reconfigure_client::action_t::honor_owner);
    return static_cast<unsigned int>(status);
}

mmio_buffer_t buffer_allocate(char* service, long size)
{
    auto afu = dynamic_pointer_cast<afu_client>(service_manager::instance()->get_service(service));
    auto buffer_ptr = afu->allocate_buffer(size, true);
    mmio_buffer_t buffer;
    buffer.virtual_addr = buffer_ptr->address();
    buffer.physical_addr = buffer_ptr->physical();
    buffer.size = buffer_ptr->size();
    return buffer;
}

void buffer_release(char* service, unsigned char* buffer)
{
    auto afu = dynamic_pointer_cast<afu_client>(service_manager::instance()->get_service(service));
    
    afu->release_buffer(buffer);
}

void mmio_write32(char* service, unsigned int address, int value)
{
    auto afu = dynamic_pointer_cast<afu_client>(service_manager::instance()->get_service(service));
    if (afu)
    {
        afu->mmio_write32(address, value);
    }
    else
    {
        std::cerr << "Could not find service with name: " << service << std::endl;
    }
}

void mmio_write64(char* service, unsigned int address, long value)
{
    auto afu = dynamic_pointer_cast<afu_client>(service_manager::instance()->get_service(service));
    if (afu)
    {
        afu->mmio_write64(address, value);
    }
    else
    {
        std::cerr << "Could not find service with name: " << service << std::endl;
    }
}

unsigned int mmio_read32(char* service, unsigned int address)
{
    auto afu = dynamic_pointer_cast<afu_client>(service_manager::instance()->get_service(service));
    if (afu)
    {
        return afu->mmio_read32(address);
    }
    else
    {
        std::cerr << "Could not find service with name: " << service << std::endl;
        return 0;
    }
}

long long mmio_read64(char* service, unsigned int address)
{
    auto afu = dynamic_pointer_cast<afu_client>(service_manager::instance()->get_service(service));
    if (afu)
    {
        return static_cast<long long>(afu->mmio_read64(address));
    }
    else
    {
        std::cerr << "Could not find service with name: " << service << std::endl;
        return 0;
    }
}

void afu_reset(char* service)
{
    auto afu = dynamic_pointer_cast<afu_client>(service_manager::instance()->get_service(service));
    if (afu)
    {
        afu->reset();
    }
    else
    {
        std::cerr << "Could not find service with name: " << service << std::endl;
    }
}

