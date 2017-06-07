#include "arguments.hpp"
#include "input/simple_work_provider.hpp"
#include "engine/engine.hpp"
#include "error.hpp"

#include <iostream>

using namespace Pipeline;
using namespace std;

int main(int argc, char * argv[])
{
    string input_file_path;
    string task_name;

    Arguments args;
    args.add_option("-in", input_file_path);
    args.add_option("-task", task_name);

    try {
        args.parse(argc, argv);
    } catch (Arguments::Error & e) {
        cerr << "Command line error: " << e.what() << endl;
        return 1;
    }

    if (input_file_path.empty())
    {
        cerr << "Missing option: -in (input_file_path)" << endl;
        return 1;
    }

    Simple_Work_Provider work_provider;
    Engine engine;

    try
    {

        work_provider.parse(input_file_path);

        if (task_name.empty())
            work_provider.request_all(&engine);
        else
            work_provider.request(task_name, &engine);

        engine.execute();
    }
    catch (Pipeline::Error & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
