#include "arguments.hpp"
#include "pipeline.hpp"
#include "input/tasks.hpp"
#include "input/persistence.hpp"
#include "engine/engine.hpp"
#include "error.hpp"
#include "util/filesystem.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace Pipeline;
using namespace std;

Options & Pipeline::options()
{
    static Options options;
    return options;
}

int main(int argc, char * argv[])
{
    string task_list_path;
    string task_generator_path;
    vector<string> task_names;

    Arguments args;
    args.add_option("-l", task_list_path);
    args.add_option("-g", task_generator_path);
    args.add_switch("-v", options().verbose, true);
    args.save_remaining(task_names);

    try {
        args.parse(argc, argv);
    } catch (Arguments::Error & e) {
        cerr << "Command line error: " << e.what() << endl;
        return 1;
    }

    if (!task_list_path.empty() && !task_generator_path.empty())
    {
        cerr << "Options -l and -g can not be used simultaneously." << endl;
        return 1;
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
            store.task_list_path.clear();
        }
        else if (!task_list_path.empty())
        {
            store.task_list_path = task_list_path;
            store.task_generator_path.clear();
        }

        task_generator_path = store.task_generator_path;
        task_list_path = store.task_list_path;

        if (!task_generator_path.empty())
        {
            if (!file_exists(task_generator_path))
                throw Error("Generator does not exist: " + task_generator_path);

            task_list_path = "pipeline.json";

            string command { "python3 -m pipeline " };
            command += task_generator_path;
            command += " ";
            command += task_list_path;

            cerr << "> Updating tasks: " << task_generator_path << endl;
            if (options().verbose)
                cerr << command << endl;

            int result = system(command.c_str());

            cerr << endl;

            if (result != 0)
                throw Error("Generator execution failed.");
        }

        Task_Manager task_manager;

        if (!task_list_path.empty())
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

        Engine engine;

        cerr << "> Scheduling tasks" << endl;

        if (task_names.empty())
        {
            task_manager.request_all(&engine);
        }
        else
        {
            for (auto & name : task_names)
                task_manager.request(name, &engine);
        }

        cerr << endl;

        engine.execute();
    }
    catch (Pipeline::Error & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
