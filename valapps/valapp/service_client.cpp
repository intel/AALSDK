#include "service_client.h"


using namespace AAL;

service_client::service_client()
    : status_(service_client::unknown)
    , serviceBase_(0)
    , releasable_(true)
{
    SetInterface(iidServiceClient, dynamic_cast<IServiceClient*>(this));
    sem_.Create(0, 1);
    register_interface<IALIMMIO>(iidALI_MMIO_Service);
    register_interface<IALIUMsg>(iidALI_UMSG_Service);
    register_interface<IALIBuffer>(iidALI_BUFF_Service);
    register_interface<IALIPerf>(iidALI_PERF_Service);
    register_interface<IALIReset>(iidALI_RSET_Service);
    register_interface<IALISignalTap>(iidALI_STAP_Service);
}

void service_client::register_interface(const std::string &ifname, btIID ifid)
{
    interface_id_map_[ifname] = ifid;
}

void service_client::release()
{
    if (0 != serviceBase_ && serviceBase_->IsOK())
    {
        releasable_ = true;
        status_ = service_client::unknown;
        serviceInterface_->Release(TransactionID());
    }
}

service_client::status_t service_client::status(bool waitForStatus)
{
    if (waitForStatus && status_ == service_client::unknown)
    {
        sem_.Wait();
    }
    return status_;
}

void service_client::serviceAllocated(IBase *pServiceBase, TransactionID const &rTransID)
{
    status_ = service_client::allocated;
    serviceBase_ = pServiceBase;
    serviceInterface_ = dynamic_ptr<IAALService>(iidService, pServiceBase);
    Log() << "Service Allocated" << std::endl;
    sem_.Post(1);
}

void service_client::serviceAllocateFailed(const AAL::IEvent &rEvent)
{
    status_ = service_client::allocate_error;
    Log() << "Service allocation failed: " << dynamic_cast<const CExceptionTransactionEvent*>(&rEvent)->Description() << std::endl;
    sem_.Post(1);
}

void service_client::serviceReleased(const AAL::TransactionID& rTransID)
{
    Log() << "Service released" << std::endl;
    serviceBase_ = 0;
    serviceInterface_ = 0;
    status_ = service_client::released;
    sem_.Post(1);
}

void service_client::serviceReleaseRequest(AAL::IBase *pServiceBase, const AAL::IEvent &rEvent)
{
    Log() << "serviceReleaseRequest" << std::endl;
    if (releasable_)
    {
        release();
    }
}

void service_client::serviceReleaseFailed(const AAL::IEvent& rEvent)
{
    Log() << "serviceReleaseFailed" << std::endl;
    status_ = service_client::status_t::release_error;
    sem_.Post(1);
}

void service_client::serviceEvent(const AAL::IEvent &rEvent)
{
    Log() << "serviceEvent" << std::endl;
}




