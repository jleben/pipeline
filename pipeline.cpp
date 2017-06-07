#include "arguments.hpp"
#include "input/job_parser.hpp"
#include "engine/job_worker.hpp"
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

    try
    {
        Job job;

        {
            Job_Parser parser;
            job = parser.parse(input_file_path);
        }

        {
            vector<string> task_names;
            if (!task_name.empty())
                task_names.push_back(task_name);

            Job_Worker worker;
            worker.execute(job, task_names);
        }
    }
    catch (Pipeline::Error & e)
    {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
