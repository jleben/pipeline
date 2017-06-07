#pragma once

#include "../work/work.hpp"

#include <unordered_set>
#include <unordered_map>

namespace Pipeline {

using std::unordered_set;
using std::unordered_map;

class Simple_Work_Provider : public Work_Provider
{
public:
    Simple_Work_Provider();
    void parse(const string & file_path);
    void request_all(Engine *) override;
    void request(const string & name, Engine *) override;

private:
    struct Task
    {
        string name;
        string command;
        vector<string> dependencies;
    };

    struct SimpleWork : public Pipeline::Work
    {
        vector<Work*> dependencies() override { return d_dependencies; }
        void execute() override;

        string name;
        string command;
        vector<Work*> d_dependencies;
    };

    Work * add_work(const string & name);

    unordered_map<string, Task> d_tasks;
    unordered_map<string, Work*> d_work;
    unordered_set<string> d_visiting;
};

}
