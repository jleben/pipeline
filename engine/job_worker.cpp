#include "job_worker.hpp"
#include "../error.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <cstdlib>

#include <sys/wait.h>
#include <unistd.h>

using namespace std;

namespace Pipeline {

class Job_Plan
{
    struct TaskState
    {
        bool visited = false;
        bool visiting = false;
    };

    unordered_map<string,const Task*> task_dictionary;
    unordered_map<string,TaskState> task_state;
    vector<const Task*> sorted_tasks;

public:
    Job_Plan(const Job & job, const vector<string> & task_names)
    {
        build_task_dictionary(job);

        if (task_names.empty())
        {
            for(auto & task : job.tasks)
            {
                if (!task_state[task.name].visited)
                    visit(&task);
            }
        }
        else
        {
            for(auto & task_name : task_names)
            {
                const Task * task = task_dictionary[task_name];
                if (!task)
                    throw Error("Task does not exist: " + task_name);

                if (!task_state[task->name].visited)
                    visit(task);
            }
        }
    }

    vector<const Task*> ordered_tasks()
    {
        return sorted_tasks;
    }

private:
    void build_task_dictionary(const Job & job)
    {
        for (auto & task : job.tasks)
        {
            bool is_unique;
            tie(ignore, is_unique) = task_dictionary.emplace(task.name, &task);
            if (!is_unique)
            {
                throw Error("Duplicate task name: " + task.name);
            }
        }
    }

    void visit(const Task * task)
    {
        auto & state = task_state[task->name];
        state.visiting = true;
        for (auto & dep_name : task->dependencies)
        {
            auto dep = task_dictionary[dep_name];
            if (!dep)
            {
                throw Error("Dependency " + dep_name + " of task " + task->name + " does not exist.");
            }
            auto & dep_state = task_state[dep->name];
            if (dep_state.visiting)
            {
                throw Error("Mutual dependency between " + task->name + " and " + dep_name);
            }
            if (!dep_state.visited)
                visit(dep);
        }
        state.visiting = false;
        state.visited = true;
        sorted_tasks.push_back(task);
    }
};

void Job_Worker::execute(const Job & job, const vector<string> & task_names)
{
    Job_Plan plan(job, task_names);

    for (const Task * task : plan.ordered_tasks())
    {
        execute(task);
    }
}

void Job_Worker::execute(const Task * task)
{
    cout << "Executing " << task->name <<  ": " << task->command << endl;

    int result = system(task->command.c_str());

    cout << endl;

    if (result != 0)
    {
        throw Error("Command for task " + task->name + " failed: " + task->command);
    }

#if 0
    vector<string> args;

    {
        istringstream stream(t->command);
        while(stream)
        {
            string arg;
            stream >> arg;
            if (stream)
                args.push_back(arg);
        }
    }

    if (args.empty())
        throw Error("Invalid command: " + t->command);

    pid_t pid = fork();
    if (pid == 0)
    {
        auto argv = new char*[args.size()];
        for (int i = 1; i < args.size(); ++i)
            argv[i-1] = strdup(args[i].c_str());
        argv[args.size()-1] = nullptr;

        execvp(args[0].c_str(), argv);
    }
    else
    {
        waitpid(pid, nullptr, 0);
    }
#endif
}

}
