#include "process.h"
#include <algorithm>
#include <future>
#include <thread>
#include <iostream>
#include <unistd.h>
#include <wait.h>
#include <signal.h>
#include <sys/types.h>

using namespace std;
using namespace std::chrono;

namespace utils
{
    process::process(int pid) : pid_(pid)
    {

    }

    process::~process()
    {

    }

    int process::wait(int timeout_msec)
    {
        // make a task to call the waitid call
        packaged_task<int()> task([this]()
                {
                    siginfo_t info;
                    int options = WEXITED | WSTOPPED;
                    waitid(P_PID, this->pid_, &info, options);
                    if (info.si_code == CLD_KILLED || info.si_code == CLD_DUMPED)
                    {
                        cerr << "Warning: child exited abnormally with code: " << info.si_status << std::endl;
                    }
                    return info.si_status;
                });

        // get the task's future value
        future<int> fv = task.get_future();
        // run the task in a detached thread (since we will wait on the future value)
        thread(move(task)).detach();
        if ( timeout_msec > -1 && fv.wait_for(milliseconds(timeout_msec)) == future_status::timeout)
        {
            return -1;
        }
        // wait on the value
        fv.wait();
        // return the value returned by the task (lambda)
        return fv.get();
    }

    void process::terminate()
    {
        kill(pid_, SIGINT);
    }

    void process::terminate(int signal)
    {
        kill(pid_, signal);
    }

    process process::start(const string &file, const vector<string> &args)
    {
        // new args are first the program name,
        // followed by the arguments
        // terminated with a null char
        char** cargs = new char*[args.size()+2];

        // set the program name
        cargs[0] = const_cast<char*>(file.c_str());

        // copy the args vector to char**
        // starting at index 1
        for (int i = 0; i < args.size(); ++i)
        {
            cargs[i+1] = const_cast<char*>(args[i].c_str());
        }

        // last element is null char
        cargs[args.size()+1] = 0;

        pid_t child = fork();
        if (child != 0)
        {
            delete []cargs;
            return process(child);
        }
        else
        {
            execvp(file.c_str(), cargs);
            // shouldn't be here
            cerr << "We shouldn't be here after exec" << endl;
            abort();
        }
    }
}
