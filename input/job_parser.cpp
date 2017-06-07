#include "job_parser.hpp"
#include "../error.hpp"
#include "../json/json.hpp"

#include <fstream>
#include <iostream>

using namespace std;

namespace Pipeline {

using json = nlohmann::json;

Job Job_Parser::parse(const string & file_path)
{
    Job job;

    json job_data;

    {
        ifstream file(file_path);
        if (!file.is_open())
            throw Error("Could not open file.");

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

            job.tasks.push_back(t);
        }
    } catch (std::domain_error & e) {
        throw Error(string("Parser: ") + e.what());
    }

    return job;
}

}
