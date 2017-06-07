#pragma once

#include "../job.hpp"

namespace Pipeline {

class Job_Worker
{
public:
    void execute(const Job &, const vector<string> & task_names);

private:
    void execute(const Task *);
};

}

