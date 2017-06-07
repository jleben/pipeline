#pragma once

#include <stdexcept>

namespace Pipeline {

class Error : public std::exception
{
    string msg;

public:
    Error() {}
    Error(const string & msg): msg(msg) {}

    const char * what() const noexcept override
    {
        return msg.c_str();
    }

};

}
