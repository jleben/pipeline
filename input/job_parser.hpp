#pragma once

#include "../job.hpp"

namespace Pipeline {

class Job_Parser
{
public:
    Job parse(const string & file_path);
};

}
