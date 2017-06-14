#pragma once

#include <string>
#include <unordered_map>

#include "../json/json.hpp"

namespace Pipeline {

using std::string;
using std::unordered_map;
using json = nlohmann::json;

class Store
{
public:
    string task_generator_path;
    string task_list_path;
    unordered_map<string,json> tasks;

    void read(const string & path);
    void write(const string & path);
};

}
