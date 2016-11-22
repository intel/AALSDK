#pragma once
#include <map>
#include <string>
#include "utils.h"

class service_client;


class client_factory
{
    public:
    typedef service_client* (*client_maker_t)();
        
    static client_factory* instance();
    
    template<class T>
    static bool register_client()
    {
        auto typestr = utils::get_typename<T>();
        client_factory::instance()->register_client(typestr, 
                []()
                { 
                    return dynamic_cast<service_client*>(new T());
                });
        return true;
    }

    inline void register_client(const std::string &client_type, client_maker_t maker)
    {
        client_makers_[client_type] = maker;
    }

    inline service_client* make(const std::string& client_type)
    {
        auto iter = client_makers_.find(client_type);
        if (iter != client_makers_.end())
        {
            return iter->second();
        }
        return 0;
    }

    private:
    static client_factory* instance_;
    std::map<std::string, client_maker_t> client_makers_;





};
