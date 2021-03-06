#pragma once

#include "../work/work.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>

namespace Pipeline {

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;
using std::set;

class Engine;
class Task;
class Store;

class File
{
public:
    string path;
    vector<Task*> producers;
    vector<Task*> consumers;
};

class Task
{
public:
    bool defined = false;
    bool changed = false;
    string name;
    string command;
    vector<File*> input_files;
    vector<File*> output_files;
    vector<Task*> upstream_tasks;
};

class Task_Work : public Work
{
public:
    Task * d_task;
    vector<Work*> d_dependencies;
    vector<Work*> dependencies() override { return d_dependencies; }
    void execute() override;
};

class Task_Manager
{
    // FIXME: Delete stuff when done using

public:
    void load_file(const string & path, Store &);
    set<string> task_names() const;
    void request_all(Engine *, bool force);
    void request(const string & name, Engine *, bool force);
    void request_matching(const string & pattern, Engine *, bool force);

private:
    File * file(const string & path);
    Task * task(const string & name);
    Task * existing_task(const string & name);
    Work * get_work(Task*, bool force);
    void add_work(Task*, Engine*, bool force);

    unordered_map<string, File*> d_files;
    unordered_map<string, Task*> d_tasks;
    unordered_map<string, Task_Work*> d_work;
};

}
