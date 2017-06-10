#pragma once

#include <string>
#include <ctime>

namespace Pipeline {

using std::string;
using std::time_t;

bool file_exists(const string & path);

bool file_is_newer(const string & path, const string & other_path);

time_t file_last_modified_time(const string & path);

}
