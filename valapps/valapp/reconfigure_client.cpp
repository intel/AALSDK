#include "reconfigure_client.h"
#include <functional>

using namespace AAL;

reconfigure_client::reconfigure_client() : service_client()
{
   SetInterface(iidALI_CONF_Service_Client, dynamic_cast<IALIReconfigure_Client*>(this));
}

reconfigure_client::~reconfigure_client()
{

}

void reconfigure_client::serviceAllocated(IBase *pServiceBase, TransactionID const &rTransID)
{
    reconfigure_if_ = dynamic_ptr<IALIReconfigure>(iidALI_CONF_Service, pServiceBase);
    service_client::serviceAllocated(pServiceBase, rTransID);
}

std::future<reconfigure_client::status_t> reconfigure_client::reconfigure_async(const std::string &bitstream, unsigned long timeout, action_t action)
{
    std::packaged_task<reconfigure_client::status_t(const std::string &, unsigned long, bool)>
        task(std::bind(&reconfigure_client::reconfigure, this, bitstream, timeout, honor_owner));
    auto fv = task.get_future();
    task(bitstream, timeout, honor_owner);
    return fv;
}

reconfigure_client::status_t reconfigure_client::reconfigure(const std::string &bitstream, unsigned long timeout, action_t action)
{
    auto action_honor = AALCONF_RECONF_ACTION_HONOR_OWNER_ID;

    switch(action)
    {
        case action_t::honor_owner : action_honor = AALCONF_RECONF_ACTION_HONOR_OWNER_ID; break;
        case action_t::honor_request : action_honor = AALCONF_RECONF_ACTION_HONOR_REQUEST_ID; break;
        default : break;
    }

    NamedValueSet nvs;
    nvs.Add(AALCONF_MILLI_TIMEOUT, static_cast<bt64bitInt>(timeout));
    nvs.Add(AALCONF_RECONF_ACTION, action_honor);
    nvs.Add(AALCONF_FILENAMEKEY, bitstream.c_str());
    nvs.Add(AALCONF_REACTIVATE_DISABLED, false);
    TransactionID tid;
    // make a packaged_task that executes the lambda
    // this lambda will call the reconfigure interface and wait until
    // a callback inserts the transaction id associated with passed into the task
    // is inserted into our transaction_id_ map
    reconfigure_if_->reconfConfigure(tid, nvs);
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk,
    [tid, this]
    {
        return this->transaction_map_.find(tid.ID()) != transaction_map_.end() ||
               this->transaction_map_.find(0) != transaction_map_.end();
    });
    auto iter = transaction_map_.find(tid.ID());
    if (iter == transaction_map_.end())
    {
        iter = transaction_map_.find(0);
    }

    if (iter != transaction_map_.end())
    {
        transaction_map_.erase(iter);
        return iter->second;
    }
    Log() << "Unknown status of reconfigure" << std::endl;
    return reconfigure_client::reconfigure_error;
}

reconfigure_client::status_t reconfigure_client::deactivate(uint64_t timeout, action_t action)
{
    auto action_honor = AALCONF_RECONF_ACTION_HONOR_OWNER_ID;

    switch(action)
    {
        case action_t::honor_owner : action_honor = AALCONF_RECONF_ACTION_HONOR_OWNER_ID; break;
        case action_t::honor_request : action_honor = AALCONF_RECONF_ACTION_HONOR_REQUEST_ID; break;
        default : break;
    }
    
    NamedValueSet nvs;
    nvs.Add(AALCONF_MILLI_TIMEOUT, static_cast<bt64bitInt>(timeout));
    nvs.Add(AALCONF_RECONF_ACTION, action_honor);
    nvs.Add(AALCONF_REACTIVATE_DISABLED, false);
    TransactionID tid;
    // make a packaged_task that executes the lambda
    // this lambda will call the reconfigure interface and wait until
    // a callback inserts the transaction id associated with passed into the task
    // is inserted into our transaction_id_ map
    reconfigure_if_->reconfDeactivate(tid, nvs);
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk,
    [tid, this]
    {
        return this->transaction_map_.find(tid.ID()) != transaction_map_.end() ||
               this->transaction_map_.find(0) != transaction_map_.end();
    });
    auto iter = transaction_map_.find(tid.ID());
    if (iter == transaction_map_.end())
    {
        iter = transaction_map_.find(0);
    }

    if (iter != transaction_map_.end())
    {
        transaction_map_.erase(iter);
        return iter->second;
    }
    Log() << "Unknown status of deactivate" << std::endl;
    return reconfigure_client::reconfigure_error;
}

std::future<reconfigure_client::status_t> reconfigure_client::deactivate_async(uint64_t timeout, action_t action)
{
    std::packaged_task<reconfigure_client::status_t(uint32_t, bool)>
        task(std::bind(&reconfigure_client::deactivate, this, timeout, honor_owner));
    auto fv = task.get_future();
    task(timeout, honor_owner);
    return fv;
}

reconfigure_client::status_t reconfigure_client::activate(uint64_t timeout, action_t action)
{
    auto action_honor = AALCONF_RECONF_ACTION_HONOR_OWNER_ID;

    switch(action)
    {
        case action_t::honor_owner : action_honor = AALCONF_RECONF_ACTION_HONOR_OWNER_ID; break;
        case action_t::honor_request : action_honor = AALCONF_RECONF_ACTION_HONOR_REQUEST_ID; break;
        default : break;
    }

    NamedValueSet nvs;
    nvs.Add(AALCONF_MILLI_TIMEOUT, static_cast<bt64bitInt>(timeout));
    nvs.Add(AALCONF_RECONF_ACTION, action_honor);
    nvs.Add(AALCONF_REACTIVATE_DISABLED, false);
    TransactionID tid;
    // make a packaged_task that executes the lambda
    // this lambda will call the reconfigure interface and wait until
    // a callback inserts the transaction id associated with passed into the task
    // is inserted into our transaction_id_ map
    reconfigure_if_->reconfActivate(tid, nvs);
    std::unique_lock<std::mutex> lk(mutex_);
    cv_.wait(lk,
    [tid, this]
    {
        return this->transaction_map_.find(tid.ID()) != transaction_map_.end() ||
               this->transaction_map_.find(0) != transaction_map_.end();
    });
    auto iter = transaction_map_.find(tid.ID());
    if (iter == transaction_map_.end())
    {
        iter = transaction_map_.find(0);
    }

    if (iter != transaction_map_.end())
    {
        transaction_map_.erase(iter);
        return iter->second;
    }
    Log() << "Unknown status of reactivate" << std::endl;
    return reconfigure_client::reconfigure_error;
}

std::future<reconfigure_client::status_t> reconfigure_client::activate_async(uint64_t timeout, action_t action)
{
    std::packaged_task<reconfigure_client::status_t(uint32_t, bool)>
        task(std::bind(&reconfigure_client::activate, this, timeout, honor_owner));
    auto fv = task.get_future();
    task(timeout, honor_owner);
    return fv;
}

// <IALIReconfigure_Client interface>
void reconfigure_client::deactivateSucceeded( TransactionID const &rTranID )
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        transaction_map_[rTranID.ID()] = status_t::deactivate_success;
    }
    cv_.notify_all();
}

void reconfigure_client::deactivateFailed( IEvent const &rEvent )
{
    IExceptionTransactionEvent *evt =
         dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
    if (0 != evt)
    {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            transaction_map_[evt->TranID().ID()] = status_t::deactivate_error;
        }
        cv_.notify_all();
    }
}

void reconfigure_client::configureSucceeded( TransactionID const &rTranID )
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        transaction_map_[rTranID.ID()] = status_t::reconfigure_success;
    }
    cv_.notify_all();
}

void reconfigure_client::configureFailed( IEvent const &rEvent )
{
    IExceptionTransactionEvent *evt =
         dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
    if (0 != evt)
    {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            transaction_map_[evt->TranID().ID()] = status_t::reconfigure_error;
        }

        cv_.notify_all();
    }
}


void reconfigure_client::activateSucceeded( TransactionID const &rTranID )
{
    {
        std::lock_guard<std::mutex> lk(mutex_);
        transaction_map_[rTranID.ID()] = status_t::activate_success;
    }
    cv_.notify_all();
}

void reconfigure_client::activateFailed( IEvent const &rEvent )
{
    IExceptionTransactionEvent *evt =
         dynamic_ptr<IExceptionTransactionEvent>(iidExTranEvent, rEvent);
    if (0 != evt)
    {
        {
            std::lock_guard<std::mutex> lk(mutex_);
            transaction_map_[evt->TranID().ID()] = status_t::activate_error;
        }
        cv_.notify_all();
    }
}
// <end IALIReconfigure_Client interface>




