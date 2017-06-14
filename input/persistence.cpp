#include "persistence.hpp"
#include "../error.hpp"
#include "../json/json.hpp"

#include <fstream>

using namespace std;

namespace Pipeline {

using json = nlohmann::json;

void Store::read(const string & path)
{
    ifstream file(path);
    if (!file.is_open())
        throw Error("Can not open store: " + path);

    json j;

    try {
        file >> j;
    } catch (std::invalid_argument & e) {
        throw Error(string("Failed to parse store: ") + e.what());
    }

    if (!j.is_object())
        throw Error(string("Invalid store format: Root is not an object."));

    try
    {
        if (!j["task-generator"].is_null())
            task_generator_path = j["task-generator"];
        if (!j["task-list"].is_null())
            task_list_path = j["task-list"];

        auto task_data = j["tasks"];
        if (!task_data.is_null())
        {
            for (auto d = task_data.begin(); d != task_data.end(); ++d)
            {
                string name = d.key();
                tasks[name] = d.value();
            }
        }
    }
    catch(std::domain_error & e)
    {
        throw Error(string("Invalid store format: ") + e.what());
    }
}

void Store::write(const string & path)
{
    ofstream file(path);
    if (!file.is_open())
        throw Error("Can not open store: " + path);

    json j = {
        { "task-generator", task_generator_path },
        { "task-list", task_list_path }
    };

    j["tasks"] = tasks;

    file << j.dump(4) << endl;
}

}
