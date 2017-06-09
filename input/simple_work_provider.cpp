#include "simple_work_provider.hpp"
#include "../pipeline.hpp"
#include "../error.hpp"
#include "../engine/engine.hpp"
#include "../json/json.hpp"

#include <fstream>
#include <cstdlib>

using namespace std;

namespace Pipeline {

Simple_Work_Provider::Simple_Work_Provider()
{

}

void Simple_Work_Provider::parse(const string & file_path)
{
    using json = nlohmann::json;

    json job_data;

    {
        ifstream file(file_path);
        if (!file.is_open())
            throw Error("Can not open task list: " + file_path);

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

            Task t;

            t.name = task_data["name"];

            t.command = task_data["command"];

            auto & deps = task_data["dependencies"];

            if (!(deps.is_string() || deps.is_array() || deps.is_null()))
                throw Error("Parser: Invalid dependencies of task " + t.name + ".");

            for (auto & input_data : task_data["dependencies"])
                t.dependencies.push_back(input_data);

            bool has_unique_name;
            tie(ignore, has_unique_name) = d_tasks.emplace(t.name, t);

            if (!has_unique_name)
                throw Error("Parser: Task name used multiple times: " + t.name);
        }
    }
    catch (std::domain_error & e)
    {
        throw Error(string("Parser: ") + e.what());
    }
}

void Simple_Work_Provider::request_all(Engine * engine)
{
    for (const auto & entry : d_tasks)
    {
        request(entry.first, engine);
    }
}

void Simple_Work_Provider::request(const string & name, Engine * engine)
{
    Work * work = add_work(name);

    engine->schedule(work);
}

Work * Simple_Work_Provider::add_work(const string & name)
{
    if (d_work.find(name) != d_work.end())
        return d_work.at(name);

    if (d_visiting.find(name) != d_visiting.end())
        throw Error("Task depends on itself: " + name);

    auto task_it = d_tasks.find(name);
    if (task_it == d_tasks.end())
    {
        throw Error("Task does not exist: " + name);
    }

    const Task & task = task_it->second;

    auto work = new SimpleWork;
    work->name = task.name;
    work->command = task.command;

    d_visiting.insert(name);

    for (auto & dep_name : task.dependencies)
    {
        auto dep_work = add_work(dep_name);
        work->d_dependencies.push_back(dep_work);
    }

    d_visiting.erase(name);

    d_work[name] = work;

    return work;
}

void Simple_Work_Provider::SimpleWork::execute()
{
    cerr << "> " << name << endl;
    if (options().verbose)
        cerr << command << endl;

    int result = system(command.c_str());

    cerr << endl;

    if (result != 0)
    {
        throw Error("Command for task " + name + " failed: " + command);
    }
}

}
