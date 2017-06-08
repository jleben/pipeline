#include "filesystem.hpp"
#include "../error.hpp"

#include <sys/stat.h>
#include <unistd.h>

namespace Pipeline {

bool file_exists(const string & path)
{
    return access(path.c_str(), R_OK) == 0;
}

bool file_is_newer(const string & a, const string & b)
{
    struct stat a_stat;
    struct stat b_stat;

    if (stat(a.c_str(), &a_stat) != 0)
        throw Error("Failed to access file: " + a);
    if (stat(b.c_str(), &b_stat) != 0)
        throw Error("Failed to access file: " + b);

    return a_stat.st_mtime > b_stat.st_mtime;
}

}
