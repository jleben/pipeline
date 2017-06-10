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

    try
    {
        Store store;

        string store_path("pipeline-store.json");

        if (file_exists(store_path))
        {
            store.read(store_path);
        }

        bool new_task_list = false;

        if (!task_list_path.empty())
        {
            if (store.task_list_path != task_list_path)
                new_task_list = true;
            store.task_list_path = task_list_path;
        }
        else if (store.task_list_path.empty())
        {
            store.task_list_path = "pipeline.json";
            new_task_list = true;
        }

        bool new_generator = false;

        if (!task_generator_path.empty())
        {
            if (task_generator_path != store.task_generator_path)
                new_generator = true;
            store.task_generator_path = task_generator_path;
        }

        store.write(store_path);

        bool has_generator = false;

        if (!store.task_generator_path.empty())
        {
            has_generator = true;

            if (!file_exists(store.task_generator_path))
                throw Error("Generator does not exist: " + store.task_generator_path);

            if (!file_exists(store.task_list_path) ||
                file_is_newer(store.task_generator_path, store.task_list_path))
            {
                new_generator = true;
            }
        }

        bool regenerate = new_generator || (new_task_list && has_generator);

        if (regenerate)
        {
            string command { "python3 " };
            command += store.task_generator_path;
            command += " ";
            command += store.task_list_path;

            cerr << "> Task list" << endl;
            if (options().verbose)
                cerr << command << endl;

            int result = system(command.c_str());

            cerr << endl;

            if (result != 0)
                throw Error("Generator execution failed.");
        }

        if (!file_exists(store.task_list_path))
        {
            ostringstream msg;
            msg << "Task list does not exist: " + store.task_list_path + ".";
            if (!has_generator)
                msg << " No task generator provided.";

            throw Error(msg.str());
        }

        Task_Manager task_manager;
        Engine engine;

        bool new_tasks = new_generator || new_task_list;

        task_manager.parse_file(store.task_list_path, new_tasks);

        if (task_names.empty())
        {
            task_manager.request_all(&engine);
        }
        else
        {
            for (auto & name : task_names)
                task_manager.request(name, &engine);
        }

        engine.execute();
    }
    catch (Pipeline::Error & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
