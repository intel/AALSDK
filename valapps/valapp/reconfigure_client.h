#include <aalsdk/AALTypes.h>
#include <aalsdk/service/IALIAFU.h>
#include <thread>
#include <future>
#include "service_client.h"

class reconfigure_client : public service_client, public AAL::IALIReconfigure_Client
{
    public:

        enum status_t
        {
            status_none,
            deactivate_success,
            deactivate_error,
            activate_success,
            activate_error,
            reconfigure_success,
            reconfigure_error
        };

        enum action_t
        {
            default_action,
            honor_owner,
            honor_request
        };

        reconfigure_client();

        virtual ~reconfigure_client();

        std::future<status_t> reconfigure_async(const std::string &bitstream_data, unsigned long timeout, action_t action); 
        status_t reconfigure(const std::string &bitstream_data, unsigned long timeout, action_t action);

        std::future<status_t> deactivate_async(uint64_t timeout, action_t action);
        status_t deactivate(uint64_t timeout, action_t action);

        std::future<status_t> activate_async(uint64_t timeout, action_t action);
        status_t activate(uint64_t timeout, action_t action);

        // <IServiceClient interface>
        void serviceAllocated(AAL::IBase *pServiceBase, AAL::TransactionID const &rTransID);
        // </IServiceClient interface>

        // <IALIReconfigure_Client interface>
        virtual void deactivateSucceeded( AAL::TransactionID const &rTranID );
        virtual void deactivateFailed( AAL::IEvent const &rEvent );

        virtual void configureSucceeded( AAL::TransactionID const &rTranID );
        virtual void configureFailed( AAL::IEvent const &rEvent );

        virtual void activateSucceeded( AAL::TransactionID const &rTranID );
        virtual void activateFailed( AAL::IEvent const &rEvent );
        // <end IALIReconfigure_Client interface>
    private:
        AAL::IALIReconfigure* reconfigure_if_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::map<AAL::btID, status_t> transaction_map_;

};
