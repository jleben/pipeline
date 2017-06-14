#pragma once

#include "../work/work.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace Pipeline {

using std::string;
using std::vector;
using std::unordered_map;
using std::unordered_set;

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
    void request_all(Engine *);
    void request(const string & name, Engine *);

private:
    File * file(const string & path);
    Task * task(const string & name);
    Work * add_work(Task*);

    unordered_map<string, File*> d_files;
    unordered_map<string, Task*> d_tasks;
    unordered_map<string, Task_Work*> d_work;
};

}
