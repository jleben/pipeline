#include "tasks.hpp"
#include "persistence.hpp"
#include "../error.hpp"
#include "../util/filesystem.hpp"
#include "../engine/engine.hpp"
#include "../json/json.hpp"
#include "../pipeline.hpp"

#include <sstream>
#include <fstream>
#include <iostream>
#include <regex>

using namespace std;

namespace Pipeline {

using json = nlohmann::json;

bool parse_string_array(json & data, vector<string> & result)
{
    if (!(data.is_string() || data.is_array() || data.is_null()))
        return false;

    for (auto & element : data)
        result.push_back(element);

    return true;
}

class Cycle_Check
{
public:
    struct Cycle
    {
        Task * task;
    };

    void check(Task * task)
    {
        visit(task);
    }

private:
    void visit(Task * task)
    {
        if (is_clear(task))
            return;

        if (is_visiting(task))
            throw Cycle({task});

        visiting.insert(task);

        for (auto input : task->input_files)
        {
            for (auto producer : input->producers)
            {
                visit(producer);
            }
        }

        for (auto upstream : task->upstream_tasks)
        {
            visit(upstream);
        }

        visiting.erase(task);

        clear.insert(task);
    }

    bool is_clear(Task * task)
    {
        return clear.find(task) != clear.end();
    }

    bool is_visiting(Task * task)
    {
        return visiting.find(task) != visiting.end();
    }

    unordered_set<Task*> clear;
    unordered_set<Task*> visiting;
};

void Task_Manager::load_file(const string & path, Store & store)
{
    json job_data;

    {
        ifstream file(path);
        if (!file.is_open())
            throw Error("Can not open task list: " + path);

        try {
            file >> job_data;
        } catch (std::invalid_argument & e) {
            throw Error(string("Parser: ") + e.what());
        }
    }

    try
    {
        for (auto & task_data : job_data)
        {
            if (!task_data.is_object())
                throw Error("Parser: Task is not an object.");

            string name = task_data["name"];

            Task * task = this->task(name);
            if (task->defined)
            {
                throw Error("Parser: Task name used multiple times: " + name);
            }

            auto old_task_data = store.tasks[name];
            if (task_data != old_task_data)
            {
                task->changed = true;
                store.tasks[name] = task_data;
            }

            task->defined = true;

            task->command = task_data["command"];

            vector<string> upstream_task_names;
            if (!parse_string_array(task_data["dependencies"], upstream_task_names))
                throw Error("Parser: Invalid dependencies of task " + task->name + ".");

            for (auto & name : upstream_task_names)
            {
                task->upstream_tasks.push_back(this->task(name));
            }

            vector<string> input_file_paths;
            if (!parse_string_array(task_data["input_files"], input_file_paths))
                throw Error("Parser: Invalid input files of task " + task->name + ".");

            for (auto & path : input_file_paths)
            {
                auto file = this->file(path);
                task->input_files.push_back(file);
                file->consumers.push_back(task);
            }

            vector<string> output_file_paths;
            if (!parse_string_array(task_data["output_files"], output_file_paths))
                throw Error("Parser: Invalid input files of task " + task->name + ".");

            for (auto & path : output_file_paths)
            {
                auto file = this->file(path);
                task->output_files.push_back(file);
                file->producers.push_back(task);
            }
        }
    }
    catch (std::domain_error & e)
    {
        throw Error(string("Parser: ") + e.what());
    }

    // Check that all tasks are defined

    for (auto & task_map : d_tasks)
    {
        Task * task = task_map.second;
        if (!task->defined)
        {
            throw Error("Task " + task->name + " required by other tasks is undefined.");
        }
    }

    // Check cycles

    Cycle_Check cycle;

    for (auto & task_map : d_tasks)
    {
        Task * task = task_map.second;
        try { cycle.check(task); }
        catch (Cycle_Check::Cycle & c)
        {
            throw Error("Task depends on itself: " + c.task->name);
        }
    }
}

File * Task_Manager::file(const string & path)
{
    // FIXME: convert path to absolute path

    File* & file = d_files[path];
    if (!file)
    {
        file = new File;
        file->path = path;
    }
    return file;
}

Task * Task_Manager::task(const string & name)
{
    Task* & task = d_tasks[name];
    if (!task)
    {
        task = new Task;
        task->name = name;
    }
    return task;
}

Task * Task_Manager::existing_task(const string & name)
{
    auto t = d_tasks.find(name);
    if (t == d_tasks.end())
        return nullptr;
    else
        return t->second;
}

void Task_Manager::request_all(Engine * engine)
{
    for (const auto & entry : d_tasks)
    {
        request(entry.first, engine);
    }
}

void Task_Manager::request(const string & name, Engine * engine)
{
    auto task = this->existing_task(name);
    if (!task)
    {
        throw Error("Task does not exist: " + name);
    }

    add_work(task, engine);
}

void Task_Manager::request_matching(const string & pattern_def, Engine * engine)
{
    bool has_match = false;

    regex pattern(pattern_def);

    for (auto entry : d_tasks)
    {
        auto & name = entry.first;
        auto & task = entry.second;
        if (regex_match(name, pattern))
        {
            add_work(task, engine);
            has_match = true;
        }
    }

    if (!has_match)
    {
        throw Error("Pattern does not match any task name: " + pattern_def);
    }
}

void Task_Manager::add_work(Task * task, Engine * engine)
{
    Work * work = get_work(task);
    if (work)
        engine->schedule(work);
}

Work * Task_Manager::get_work(Task * task)
{
    if (!task)
        throw Error("Attempt to add work for null task.");

    if (d_work.find(task->name) != d_work.end())
        return d_work.at(task->name);

    bool needs_work = false;
    vector<Work*> upstream_work;

    if (task->changed)
    {
        if (!needs_work && options().verbose)
        {
            cerr << "Task " << task->name
                 << " scheduled, because of new task definition."
                 << endl;
        }
        needs_work = true;
    }

    if (task->output_files.empty())
    {
        if (!needs_work && options().verbose)
        {
            cerr << "Task " << task->name
                 << " always scheduled, because it has no declared outputs."
                 << endl;
        }
        needs_work = true;
    }

    for (Task * upstream : task->upstream_tasks)
    {
        auto work = get_work(upstream);
        if (work)
        {
            if (!needs_work && options().verbose)
            {
                cerr << "Task " << task->name
                     << " scheduled, because upstream task "
                     << upstream->name
                     << " is scheduled."
                     << endl;
            }
            needs_work = true;
            upstream_work.push_back(work);
        }
    }

    for (File * file : task->input_files)
    {
        if (!file->producers.empty())
        {
            for (Task * upstream : file->producers)
            {
                auto work = get_work(upstream);
                if (work)
                {
                    if (!needs_work && options().verbose)
                    {
                        cerr << "Task " << task->name
                             << " scheduled, because an input file producer is scheduled."
                             << endl;
                    }
                    needs_work = true;
                    upstream_work.push_back(work);
                }
            }
        }
        else if (!file_exists(file->path))
        {
            ostringstream msg;
            msg << "File required by task " << task->name
                   << " does not exist,"
                   << " and no task produces it: "
                   << file->path;
            throw Error(msg.str());
        }
    }

    if (!needs_work)
    {
        time_t latest_input_time = 0;
        File * latest_input = nullptr;
        for (File * file : task->input_files)
        {
            auto time = file_last_modified_time(file->path);
            if (time > latest_input_time)
            {
                latest_input = file;
                latest_input_time = time;
            }
        }
        for (File * file : task->output_files)
        {
            if (!file_exists(file->path))
            {
                if (!needs_work && options().verbose)
                {
                    cerr << "Task " << task->name
                         << " scheduled, because an output file does not exist: "
                         << file->path
                         << endl;
                }
                needs_work = true;
                break;
            }
            else
            {
                auto time = file_last_modified_time(file->path);
                if (time < latest_input_time)
                {
                    if (!needs_work && options().verbose)
                    {
                        cerr << "Task " << task->name
                             << " scheduled, because output file "
                             << file->path
                             << " is older than input file: "
                             << latest_input->path
                             << endl;
                    }
                    needs_work = true;
                    break;
                }
            }
        }
    }

    if (!needs_work)
    {
        d_work[task->name] = nullptr;
        return nullptr;
    }

    auto work = new Task_Work;
    work->d_task = task;
    work->d_dependencies = upstream_work;

    d_work[task->name] = work;

    return work;
}

void Task_Work::execute()
{
    cerr << "> " << d_task->name << endl;
    if (options().verbose)
        cerr << d_task->command << endl;

    int result = system(d_task->command.c_str());

    cerr << endl;

    if (result != 0)
    {
        throw Error("Command for task " + d_task->name + " failed: " + d_task->command);
    }
}

}
