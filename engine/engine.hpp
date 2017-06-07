#pragma once

#include <unordered_set>

namespace Pipeline {

using std::unordered_set;

class Work;

class Engine
{
public:
    void schedule(Work *);
    void execute();

private:
    unordered_set<Work*> d_work;
};

}

