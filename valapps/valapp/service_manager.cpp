#include "service_manager.h"
#include "service_client.h"
#include "reconfigure_client.h"
#include "client_factory.h"
#include "afu_client.h"
#include "afu_register.h"
#include "json/json.h"
#include <string>
#include <cxxabi.h>

using namespace std;
using namespace AAL;

service_manager::ptr_t service_manager::instance_ = service_manager::ptr_t(0);

service_manager::service_manager()
    : runtime_(0)
    , env_(hwenv_t::hw)
    , runtimeStarted_(false)
{
    sem_.Create(0,1);
}

service_manager::~service_manager()
{
    shutdown();
}


service_manager::ptr_t
service_manager::instance(hwenv_t env)
{
    if (instance_ == 0)
    {
        instance_.reset(new service_manager());
        instance_->start(env);
    }
    return instance_;
}

void
service_manager::start(hwenv_t env)
{
    if (0 == runtime_)
    {
        env_ = env;
        runtime_ = new Runtime(this);
        SetInterface(iidRuntimeClient, dynamic_cast<IRuntimeClient*>(this));

        NamedValueSet configArgs, configRecord;
        if (env == hwenv_t::hw)
        {
            configRecord.Add(AALRUNTIME_CONFIG_BROKER_SERVICE, "librrmbroker");
            configArgs.Add(AALRUNTIME_CONFIG_RECORD, &configRecord);
        }

        if (!runtime_->start(configArgs))
        {
            Log() << "error starting runtime" << std::endl;
            return;
        }
        sem_.Wait();
    }
}

void
service_manager::shutdown()
{
    if (runtimeStarted_)
    {
        for(auto it : service_list_)
        {
            it->release();
        }
        runtime_->stop();
        sem_.Wait();
        delete runtime_;
        runtime_ = 0;
        runtimeStarted_ = false;
        service_list_.clear();
        service_map_.clear();
    }
}

void service_manager::define_services(const string &configFile)
{
    Json::Value root;
    Json::Reader reader;
    std::ifstream stream(configFile);
    if (reader.parse(stream, root))
    {
        for (auto service_info : root["services"])
        {
            std::string alias = service_info["alias"].asString();
            std::string name = service_info["service_lib"].asString();
            std::string afuid = service_info["afu_id"].asString();
            std::string client_type = service_info.get("client_type", "afu_client").asString();
            NamedValueSet configRecord;
            configRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, name.c_str());

            if (service_info.get("include_aia", false).asBool() && env_ == hwenv_t::hw)
            {
                configRecord.Add(keyRegAFU_ID, afuid.c_str());
                configRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");
            }
            else
            {
                configRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);
            }


            for (auto kvpair : { make_pair("bus", keyRegBusNumber),
                                 make_pair("device", keyRegDeviceNumber),
                                 make_pair("function", keyRegFunctionNumber) })
            {
                if (service_info.isMember(kvpair.first))
                {
                    auto str_value = service_info[kvpair.first].asString();
                    auto value = static_cast<btUnsigned32bitInt>(std::stoi(str_value, nullptr, 16));
                    configRecord.Add(kvpair.first, value);
                }
            }

            if (service_info.isMember("socket_id"))
            {
               auto str_value = service_info["socket_id"].asString();
               auto value = static_cast<btUnsigned32bitInt>(std::stoi(str_value, nullptr, 16));
               configRecord.Add(keyRegSocketNumber, value);
            }
            service_configs_[alias] = make_pair(configRecord, client_type);
        }
    }
    else
    {
        std::cerr << reader.getFormattedErrorMessages() << std::endl;
    }
}

service_client::ptr_t service_manager::create_service(const string &service_name)
{
    service_client::ptr_t client;
    auto iter = service_configs_.find(service_name);
    if (runtimeStarted_ && iter != service_configs_.end())
    {
        client = allocate_service(iter->first, iter->second.first, iter->second.second);
        service_map_[iter->first] = client;
        service_list_.push_back(client);
    }
    return client;
}

void service_manager::create_services(const string &configFile)
{
    if (runtimeStarted_)
    {
        Log() << "Runtime has not started. Cannot create services" << std::endl;
        return;
    }
    Json::Value root;
    Json::Reader reader;
    std::ifstream stream(configFile);
    if (reader.parse(stream, root))
    {
        auto services = root["services"];
        for(auto item : services)
        {
            auto name = item["service_lib"].asString();
            auto alias = item.get("alias", name).asString();
            auto afuid = item["afu_id"].asString();
            auto include_aia = item["include_aia"].asBool();

            NamedValueSet configRecord;
            configRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, name.c_str());
            configRecord.Add(keyRegAFU_ID, afuid.c_str());
            if (include_aia)
            {
                configRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");
            }
            else
            {
                configRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);
            }

            for (auto kvpair : { make_pair("bus", keyRegBusNumber),
                                 make_pair("device", keyRegDeviceNumber),
                                 make_pair("function", keyRegFunctionNumber) })
            {
                if (item.isMember(kvpair.first))
                {
                    auto str_value = item[kvpair.first].asString();
                    auto value = static_cast<btUnsigned32bitInt>(std::stoi(str_value, nullptr, 16));
                    configRecord.Add(kvpair.first, value);
                }
            }

            auto client = this->allocate_service(alias, configRecord);
            service_map_[name] = client;
            service_map_[alias] = client;
            service_list_.push_back(client);
            auto status = client->status();
            for (auto iface : item["interfaces"])
            {
                auto ifname = iface[0].asString();
                auto ifid_str = iface[1].asString();
                interface_map_[ifname] = client;
                btIID idnum = std::strtol(ifid_str.c_str(), nullptr, 16);
                id_map_[idnum] = client;
                client->register_interface(ifname, idnum);
            }

            if (item.isMember("registers"))
            {
                auto afu = dynamic_pointer_cast<afu_client>(client);
                if (0 == afu)
                {
                    Log() << "client is not an afu_client. Will not create register map" << std::endl;
                }
                else
                {
                    for (auto reg : item["registers"])
                    {
                        afu->add_register( afu_register( reg["id"].asString(),
                                                         reg["offset"].asString(),
                                                         reg["type"].asString(),
                                                         reg["width"].asString(),
                                                         reg["comment"].asString()));
                    }
                }
            }
        }
    }
    else
    {
        Log() << "Could not parse document: " << reader.getFormattedErrorMessages() << std::endl;
    }
}

void service_manager::release_service(const std::string &name)
{
    auto map_iter = service_map_.find(name);
    if (map_iter != service_map_.end())
    {
        auto service = map_iter->second;
        service->release();
        auto vector_iter = find(service_list_.begin(),
                                service_list_.end(),
                                service);
        if (vector_iter != service_list_.end())
        {
            service_list_.erase(vector_iter);
        }
        service_map_.erase(map_iter);
    }
}

service_client::ptr_t
service_manager::allocate_service(const string &serviceName, const string &serviceAlias, const string &afuid, bool useAia, bool useSw)
{
    NamedValueSet configRecord;
    configRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_SERVICE_NAME, serviceName.c_str());
    configRecord.Add(keyRegAFU_ID, afuid.c_str());
    if (useAia)
    {
        configRecord.Add(AAL_FACTORY_CREATE_CONFIGRECORD_FULL_AIA_NAME, "libaia");
    }
    else
    {
        configRecord.Add(AAL_FACTORY_CREATE_SOFTWARE_SERVICE, true);
    }
    return allocate_service(serviceAlias, configRecord);
}

service_client::ptr_t
service_manager::allocate_service(const std::string &serviceAlias, NamedValueSet & configRecord, std::string client_type)
{
    NamedValueSet manifest;

    manifest.Add(AAL_FACTORY_CREATE_CONFIGRECORD_INCLUDED, &configRecord);
    manifest.Add(AAL_FACTORY_CREATE_SERVICENAME, serviceAlias.c_str());

    Log() << "Allocating service " << serviceAlias << std::endl;

    service_client::ptr_t client(0);
    if (serviceAlias == "PR")
    {
        client.reset(new reconfigure_client());
    }
    else
    {
        service_client* client_ptr = client_factory::instance()->make(client_type);
        if (client_ptr == 0)
        {
            client.reset(new afu_client());
        }
        else
        {
            client.reset(client_ptr);
        }
    }
    IBase* pBase = dynamic_cast<IBase*>(client.get());

    switch(env_)
    {
        case hwenv_t::ase:
            manifest.Add(keyRegHandle, 20);
            manifest.Add(ALIAFU_NVS_KEY_TARGET, ali_afu_ase);
            break;
        default:
            break;
    }

    runtime_->allocService(pBase, manifest);
    //auto status = client->status();
    return client;
}

void service_manager::runtimeStarted(IRuntime *pRuntime,
                             const NamedValueSet &rConfigParms)
{
    Log() << "Runtime started" << std::endl;
    runtimeStarted_ = true;
    sem_.Post(1);
}

void service_manager::runtimeStopped(IRuntime *pRuntime)
{
    Log() << "Runtime stopped" << std::endl;
    runtimeStarted_ = false;
    sem_.Post(1);
}

void service_manager::runtimeStartFailed(const IEvent &rEvent)
{
    Log() << "Runtime start failed" << std::endl;
    sem_.Post(1);
    //PrintExceptionDescription(rEvent);
}

void service_manager::runtimeStopFailed(const IEvent &rEvent)
{
    Log() << "Runtime stop failed" << std::endl;
    runtimeStarted_ = false;
}

void service_manager::runtimeAllocateServiceFailed( IEvent const &rEvent)
{
    const CExceptionTransactionEvent* xevent = dynamic_cast<const CExceptionTransactionEvent*>(&rEvent);
    PrintExceptionDescription(rEvent);
    Log() << "Runtime AllocateService failed: " << xevent->Description() << std::endl;
}

void service_manager::runtimeAllocateServiceSucceeded(IBase*, TransactionID const&)
{

}

void service_manager::runtimeEvent(const IEvent &rEvent)
{

}


