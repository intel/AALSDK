/// @file service_client.h
/// @brief service_client.h contains the service_client class
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
#include <memory>

#include "Loggable.h"
#include "utils.h"


/// @brief Derives from AAL class CAASBase and implements
/// AAL interface class IServiceClient.
class service_client : public AAL::IServiceClient, 
                       public AAL::CAASBase,
                       public Loggable
{
    public:
        typedef void (* callback_t)(AAL::IBase *pServiceBase, AAL::TransactionID const &rTransID);
        typedef std::shared_ptr<service_client> ptr_t;

        enum status_t
        {
            unknown = 0,
            allocated,
            allocate_error,
            released,
            release_error
        };

        service_client();
        virtual ~service_client(){}

        template<class T>
        T* get_interface()
        {
            auto ifname = utils::get_typename<T>(); 
            return get_interface<T>(ifname);
        }

        template<class T>
        T* get_interface(const std::string &ifname)
        {
            auto ifid_iter = interface_id_map_.find(ifname);
            if (ifid_iter == interface_id_map_.end())
            {
                Log() << "Could not find interface with name: " << ifname << std::endl;
                return 0;
            }
            return get_interface<T>(ifid_iter->second);
        }

        template<class T> 
        T* get_interface(AAL::btIID serviceName)
        {
            ASSERT(serviceBase_ != 0);
            return AAL::dynamic_ptr<T>(serviceName, serviceBase_);
        }
        
        template<class T>
        void register_interface(AAL::btIID ifid)
        {
            auto ifname = utils::get_typename<T>();
            register_interface(ifname, ifid);
        }

        virtual void register_interface(const std::string &ifname, AAL::btIID ifid);
        
        virtual void release();

        inline virtual void set_releasable(bool releasable)
        {
            releasable_ = releasable;
        }

        virtual status_t status(bool waitForStatus=true);

    protected:
        // <begin IServiceClient interface>
        virtual void serviceAllocated(AAL::IBase *pServiceBase, AAL::TransactionID const &rTransID);
        virtual void serviceAllocateFailed(const AAL::IEvent &rEvent);
        virtual void serviceReleased(const AAL::TransactionID& rTransID);
        virtual void serviceReleaseRequest(AAL::IBase *pServiceBase, const AAL::IEvent &rEvent);
        virtual void serviceReleaseFailed(const AAL::IEvent& rEvent);
        virtual void serviceEvent(const AAL::IEvent &rEvent);

    private:
        status_t status_;
        AAL::IBase *serviceBase_;
        AAL::IAALService *serviceInterface_;
        AAL::CSemaphore sem_;
        std::map<std::string, AAL::btIID> interface_id_map_;
        bool releasable_;
        
};
