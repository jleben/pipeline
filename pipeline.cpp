#include "arguments.hpp"
#include "input/simple_work_provider.hpp"
#include "engine/engine.hpp"
#include "error.hpp"

#include <iostream>
#include <fstream>

using namespace Pipeline;
using namespace std;

int main(int argc, char * argv[])
{
    string input_file_path;
    vector<string> task_names;

    Arguments args;
    args.add_option("-in", input_file_path);
    args.save_remaining(task_names);

    try {
        args.parse(argc, argv);
    } catch (Arguments::Error & e) {
        cerr << "Command line error: " << e.what() << endl;
        return 1;
    }

    if (input_file_path.empty())
    {
        string path("pipeline.json");
        ifstream file(path);
        if (file.is_open())
            input_file_path = path;
    }
    if (input_file_path.empty())
    {
        string path(".pipeline.json");
        ifstream file(path);
        if (file.is_open())
            input_file_path = path;
    }
    if (input_file_path.empty())
    {
        cerr << "No pipeline.json or .pipeline.json found, and no -in option given." << endl;
        return 1;
    }

    Simple_Work_Provider work_provider;
    Engine engine;

    try
    {

        work_provider.parse(input_file_path);

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
