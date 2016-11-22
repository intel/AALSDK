#include "test_context.h"
#include "afu_test.h"
#include "arguments.h"
#include "test_manager.h"

test_context::test_context()
{

}

test_context::~test_context()
{

}

arguments& test_context::args()
{
    return test_manager::instance()->args();
}

service_client::ptr_t test_context::get_service(const std::string &service_name)
{
    auto it = service_map_.find(service_name);
    if (it == service_map_.end() || it->second->status() == service_client::status_t::released)
    {
        auto manager = service_manager::instance();
        auto client = manager->get_service(service_name, true);
        TEST_ERROR(!client, "Could not get service: ", service_name);
        client->status();
        service_map_[service_name] = client;
        return client;
    }
    return service_map_[service_name];
}
