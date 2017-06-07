#pragma once

#include <stdexcept>
#include <string>

namespace Pipeline {

using std::string;

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
