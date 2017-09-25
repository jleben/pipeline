#pragma once

namespace Pipeline {

class Options
{
public:
    bool verbose { false };
    bool force { false };
};

Options & options();

}
