#pragma once
#include <iostream>

class Loggable
{
    public:
        Loggable(std::ostream& stream = std::cout) : log_(stream) {}
    protected:
        std::ostream & Log()
        {
            return log_;
        }

       std::ostream & Log() const
       {
           return log_;
       }

    private:
        std::ostream &log_;
};
