#include "arguments.hpp"
#include "input/simple_work_provider.hpp"
#include "input/persistence.hpp"
#include "engine/engine.hpp"
#include "error.hpp"
#include "util/filesystem.hpp"

#include <iostream>
#include <fstream>

using namespace Pipeline;
using namespace std;

int main(int argc, char * argv[])
{
    string task_list_path;
    string task_generator_path;
    vector<string> task_names;

    Arguments args;
    args.add_option("-l", task_list_path);
    args.add_option("-g", task_generator_path);
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

        if (!task_list_path.empty())
            store.task_list_path = task_list_path;
        else if (store.task_list_path.empty())
            store.task_list_path = "pipeline.json";

        bool new_generator = false;

        if (!task_generator_path.empty())
        {
            if (task_generator_path != store.task_generator_path)
                new_generator = true;
            store.task_generator_path = task_generator_path;
        }

        store.write(store_path);

        if (!store.task_generator_path.empty())
        {
            if (!file_exists(store.task_generator_path))
                throw Error("Generator does not exist: " + store.task_generator_path);

            if (!file_exists(store.task_list_path) ||
                file_is_newer(store.task_generator_path, store.task_list_path))
            {
                new_generator = true;
            }
        }

        if (new_generator)
        {
            string command { "python3 " + store.task_generator_path };
            int result = system(command.c_str());
            if (result != 0)
                throw Error("Generator execution failed.");
        }

        if (!file_exists(store.task_list_path))
        {
            throw Error("Task list does not exist: " + store.task_list_path);
        }

        Simple_Work_Provider work_provider;
        Engine engine;

        work_provider.parse(store.task_list_path);

        if (task_names.empty())
        {
            work_provider.request_all(&engine);
        }
        else
        {
            for (auto & name : task_names)
                work_provider.request(name, &engine);
        }

        engine.execute();
    }
    catch (Pipeline::Error & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
