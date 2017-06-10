#include "engine.hpp"
#include "../error.hpp"
#include "../work/work.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace std;

namespace Pipeline {

class Schedule
{
    struct WorkState
    {
        bool visited = false;
        bool visiting = false;
    };

    unordered_map<Work*,WorkState> work_state;
    vector<Work*> sorted_work;

public:
    Schedule(const unordered_set<Work*> & work)
    {
        for(Work * item : work)
        {
            if (!state(item).visited)
                visit(item);
        }
    }

    WorkState & state(Work * work)
    {
        return work_state[work];
    }

    vector<Work*> ordered_work()
    {
        return sorted_work;
    }

private:
    void visit(Work * work)
    {
        auto & work_state = state(work);
        work_state.visiting = true;
        for (Work * dep : work->dependencies())
        {
            auto & dep_state = state(dep);
            if (dep_state.visiting)
            {
                throw Error("Dependency cycle among work items.");
            }
            if (!dep_state.visited)
                visit(dep);
        }
        work_state.visiting = false;
        work_state.visited = true;
        sorted_work.push_back(work);
    }
};

void Engine::schedule(Work * work)
{
    if (!work)
        throw Error("Attempt at scheduling null work.");

    d_work.insert(work);
}

void Engine::execute()
{
    Schedule schedule(d_work);

    d_work.clear();

    for (Work * work : schedule.ordered_work())
    {
        work->execute();
    }
}

}
