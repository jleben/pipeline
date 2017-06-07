#pragma once

#include <vector>
#include <string>

namespace Pipeline {

using std::vector;
using std::string;

class Engine;

class Work
{
public:
    virtual vector<Work*> dependencies() = 0;
    virtual void execute() = 0;
};

class Work_Provider
{
    virtual void request_all(Engine *) = 0;
    virtual void request(const string & name, Engine *) = 0;
};

}
