#pragma once

#include <string>
#include <vector>

namespace Pipeline {

using std::string;
using std::vector;

class Task
{
public:
    string name;
    vector<string> dependencies;
    string command;
};

}
