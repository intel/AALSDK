#include "client_factory.h"


client_factory* client_factory::instance_ = 0;

client_factory*
client_factory::instance()
{
    if (instance_ == 0)
    {
        instance_ = new client_factory();
    }
    return instance_;
}

