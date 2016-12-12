/// @brief test_context.h contains the test_context class
#pragma once
#include <memory>
#include <map>
#include <string>

#include "service_manager.h"
#include "service_client.h"
#include "arguments.h"
#include "Loggable.h"

/// @brief test_context is a convenience class used to call 
/// the service_manager singleton to get service_client ojbects.
class test_context : Loggable
{
    public:
        typedef std::shared_ptr<test_context> ptr_t;
        test_context();
        virtual ~test_context();
        
        arguments &args();

        service_client::ptr_t get_service(const std::string &service_name);

        service_client::ptr_t create_service(const std::string &service_name, const service_manager::pci_info_t & pci);

        /// @brief Get a shared_ptr to a service_client
        /// @tparam T the derived class type of service_client
        /// @param service_name The friendly name of the service 
        /// (as specified in the services JSON input file)
        template<class T>
        std::shared_ptr<T> get_service(const std::string &service_name)
        {
            auto service_ptr = get_service(service_name);
            return std::dynamic_pointer_cast<T>(service_ptr);
        }

        template<class T> 
        T* get_interface(const std::string &service_name)
        {
            auto client = get_service(service_name);
            if (0 == client)
            {
                Log() << "No service found for name: " << service_name << std::endl;
                return 0;
            }
            return client->get_interface<T>();
        }

    private:
        std::map<std::string, service_client::ptr_t> service_map_;


};
