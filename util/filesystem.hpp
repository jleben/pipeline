#pragma once

#include <string>

namespace Pipeline {

using std::string;

bool file_exists(const string & path);

bool file_is_newer(const string & path, const string & other_path);

}
