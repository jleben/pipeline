#include "util/arguments/arguments.hpp"
#include "pipeline.hpp"
#include "input/tasks.hpp"
#include "input/persistence.hpp"
#include "engine/engine.hpp"
#include "error.hpp"
#include "util/filesystem.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

using namespace Pipeline;
using namespace std;

Options & Pipeline::options()
{
    static Options options;
    return options;
}

static void print_task_names(Task_Manager & task_manager);

int main(int argc, char * argv[])
{
    string task_generator_path = "tasks.py";
    const string task_list_path = ".pipeline.json";
    vector<string> generator_args;

    bool print_help = false;
    bool list_tasks = false;

    Arguments::Parser args;
    args.add_option("-g", task_generator_path, "Task definition file. (Default = tasks.py)");
    args.add_switch("-v", options().verbose, true, "Print verbose information about what's happening.");
    args.add_switch("-f", options().force, true, "Force execution of all tasks.");
    args.add_switch("-h", print_help, true, "Print help and do nothing else.");
    args.add_switch("--list-tasks", list_tasks, true, "List tasks without executing them.");
    args.remaining_arguments(generator_args);

    try {
        args.parse(argc, argv);
    } catch (Arguments::Parser::Error & e) {
        cerr << "Command line error: " << e.what() << endl;
        return 1;
    }

    if (print_help)
    {
        args.print(cerr);
        return 0;
    }

    try
    {
        Store store;

        string store_path("pipeline-store.json");

        if (file_exists(store_path))
        {
            store.read(store_path);
        }

        if (!task_generator_path.empty())
        {
            store.task_generator_path = task_generator_path;
        }

        // Run task generator
        {
            if (!file_exists(task_generator_path))
                throw Error("Generator does not exist: " + task_generator_path);

            string command { "python3 -m pipeline " };
            command += task_generator_path;
            command += " ";
            command += task_list_path;
            for (auto & arg : generator_args)
            {
                command += " " + arg;
            }

            cerr << "> Updating tasks: " << task_generator_path << endl;
            if (options().verbose)
                cerr << command << endl;

            int result = system(command.c_str());

            cerr << endl;

            if (result != 0)
                throw Error("Generator execution failed.");
        }

        Task_Manager task_manager;

        // Load task list
        {
            cerr << "> Loading task list: " << task_list_path << endl;

            if (!file_exists(task_list_path))
            {
                ostringstream msg;
                msg << "Task list does not exist: " + task_list_path + ".";
                throw Error(msg.str());
            }

            task_manager.load_file(task_list_path, store);

            cerr << endl;
        }

        store.write(store_path);

        if (list_tasks)
        {
            print_task_names(task_manager);
            return 0;
        }

        Engine engine;

        cerr << "> Scheduling tasks" << endl;

        task_manager.request_all(&engine, options().force);

        cerr << endl;

        engine.execute();
    }
    catch (Pipeline::Error & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}

static
void print_task_names(Task_Manager & task_manager)
{
    auto available_names = task_manager.task_names();

    for (auto & name : available_names)
    {
        cout << name << endl;
    }
}
