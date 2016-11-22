#pragma once
#include <memory>
#include <chrono>
#include <string>
#include <vector>

namespace utils
{
    class process
    {
        public:
            ~process();
            typedef std::shared_ptr<process> ptr_t;
            static process start(const std::string &file,
                                 const std::vector<std::string> &args);

            
            int wait(int timeout_msec = -1);
            void terminate(int signal);
            void terminate();
        private:
            process(int pid);
            int pid_;

    };
  

} // end namespace utils
