#pragma once

#include <string>

namespace Pipeline {

using std::string;

class Store
{
public:
    string task_generator_path;
    string task_list_path;

    void read(const string & path);
    void write(const string & path);
};

}
