#pragma once

#include <vector>
#include <string>

namespace Pipeline {

using std::vector;
using std::string;

class Work
{
public:
    virtual ~Work() {}
    virtual vector<Work*> dependencies() = 0;
    virtual void execute() = 0;
};

}
