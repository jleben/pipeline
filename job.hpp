#pragma once

#include "task.hpp"

#include <vector>

namespace Pipeline {

using std::vector;

class Job
{
public:
    vector<Task> tasks;
};

}
