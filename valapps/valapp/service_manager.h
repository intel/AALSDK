/// @file service_manager.h
/// @brief service_manager.h contains the service_manager class
#pragma once

#include <aalsdk/AALTypes.h>
#include <aalsdk/Runtime.h>
#include <aalsdk/AALLoggerExtern.h>
#include <aalsdk/service/IALIAFU.h>
#include <aalsdk/AALIDDefs.h>
#include <aalsdk/CAALBase.h>

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <memory>

#include "Loggable.h"
#include "service_client.h"

/// @brief singleton class and entry point into AAL runtime.
class service_manager : public AAL::CAASBase,
                        public AAL::IRuntimeClient,
                        public Loggable
{
public:
    typedef std::shared_ptr<service_manager> ptr_t;
    typedef std::map<std::string, AAL::btUnsigned32bitInt> pci_info_t;

    enum hwenv_t
    {
        hw = 0,
        ase = 1
    };

    /// @brief Get the instance to service_manager 
    /// @returns A shared_ptr to the only service_manager instance. 
    static service_manager::ptr_t instance(hwenv_t env = hwenv_t::hw);
    ~service_manager();

    /// @brief Starts the service_manager by starting the AAL runtime
    void start(hwenv_t env = hw);

    /// @brief Shutsdown the service_manager by shutting down any service_client
    /// instances created by the service_manager and by shutting down the AAL runtime
    /// as well
    void shutdown();

    /// @brief Stores AAL seervice parameters in an internal structure using
    /// AAL data structures.
    ///
    /// @param[in] A JSON formatted congiruration file that describes the services.
    void define_services(const std::string &configFile);

    /// @brief Create an AAL service identified by the alias that maps back to 
    /// to an entry in ther services file used when calling define_services.
    /// @param[in] The friendly name of the service.
    /// @param[in] The pci info map (bus, device, function)
    ///            This overrides any bus info in the JSON file
    /// @returns A shared_ptr to a service_client.
    service_client::ptr_t create_service(const std::string &service_name, const pci_info_t &pci = pci_info_t());


    template<class T>
    T* get_interface(AAL::btIID iid)
    {
        auto it = id_map_.find(iid);
        if (it == id_map_.end())
        {
            return 0;
        }
        return it->second->get_interface<T>(iid);
    }

    template<class T>
    T* get_interface(const std::string &service_name)
    {
        auto client = get_service(service_name);
        return client->get_interface<T>();
    }

    service_client::ptr_t
    get_service(const std::string &name, bool create = false)
    {
        service_client::ptr_t ptr;
        auto it = service_map_.find(name);
        if (it == service_map_.end() || it->second->status() == service_client::status_t::released)
        {
            return create ? create_service(name) : ptr;
        }
        return it->second;
    }

    template<class T>
    T get_service(const std::string &name)
    {
        auto client = get_service(name);
        return std::dynamic_pointer_cast<T>(client);
    }

    bool have_service(const std::string &name)
    {
        return service_map_.find(name) != service_map_.end();
    }

    void release_service(const std::string& name);
protected:
    service_client::ptr_t allocate_service(const std::string &serviceName,
                                           const std::string &serviceAlias,
                                           const std::string &afuid,
                                           bool useAia=true,
                                           bool useSw=false);

    service_client::ptr_t allocate_service(const std::string &serviceAlias, AAL::NamedValueSet &nvs, std::string client_type = "afu_client");

    // <begin IRuntimeClient interface>
    void runtimeCreateOrGetProxyFailed(AAL::IEvent const &rEvent){};    // Not Used

    void runtimeStarted(AAL::IRuntime *pRuntime,
                        const AAL::NamedValueSet &rConfigParms);

    void runtimeStopped(AAL::IRuntime *pRuntime);

    void runtimeStartFailed(const AAL::IEvent &rEvent);

    void runtimeStopFailed(const AAL::IEvent &rEvent);

    void runtimeAllocateServiceFailed(AAL::IEvent const &rEvent);

    void runtimeAllocateServiceSucceeded(AAL::IBase *pClient,
                                         AAL::TransactionID const &rTranID);

    void runtimeEvent(const AAL::IEvent &rEvent);
    // <end IRuntimeClient interface>

private:
    service_manager();
    hwenv_t env_;
    AAL::Runtime* runtime_;
    bool runtimeStarted_;
    static service_manager::ptr_t instance_;
    AAL::CSemaphore sem_;
    std::map<std::string, service_client::ptr_t> interface_map_;
    std::map<std::string, service_client::ptr_t> service_map_;
    std::list<service_client::ptr_t> service_list_;
    std::map<AAL::btIID, service_client::ptr_t> id_map_;
    std::map<std::string, std::pair<AAL::NamedValueSet, std::string>> service_configs_;
};
