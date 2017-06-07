#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>

namespace Pipeline {

using std::string;
using std::istringstream;
using std::unordered_map;
using std::function;

class Arguments
{
public:
    class Error : public std::exception
    {
        string msg;
    public:
        Error() {}
        Error(const string & msg): msg(msg) {}
        const char * what() const noexcept override
        {
            return msg.c_str();
        }
    };

    struct Invalid_Option_Value : public Error
    {
        Invalid_Option_Value(const string & o, const string & v):
            Error("Invalid value '" + v + "' for option " + o) {}
    };

    struct Missing_Option_Value : public Error
    {
        Missing_Option_Value(const string & o):
            Error("Missing value for option " + o) {}
    };

    struct Redundant_Option_Value : public Error
    {
        Redundant_Option_Value(const string & o):
            Error("Redundant value for option " + o) {}
    };

    struct Option_Value_Error {};

    using Parser_Function = function<void(const string &)>;

    template <typename T>
    static
    void parse_value(const string & text, T & value)
    {
        istringstream text_stream(text);
        text_stream >> value;
        if (!text_stream || text_stream.tellg() < text.size())
            throw Option_Value_Error();
    }

    template <typename T>
    void add_option(const string & name, T & destination, const string & description = string())
    {
        auto parser = [name, &destination](const string & value)
        {
            if (value.empty())
                throw Missing_Option_Value(name);

            try {
                parse_value(value, destination);
            }
            catch (Option_Value_Error &) {
                throw Invalid_Option_Value(name, value);
            }
        };

        option_parsers.emplace(name, parser);
    }

    void add_option(const string & name, string & destination, const string & description = string())
    {
        auto parser = [name, &destination](const string & value)
        {
            if (value.empty())
                throw Missing_Option_Value(name);

            destination = value;
        };

        option_parsers.emplace(name, parser);
    }

    void add_switch(const string & name, bool & destination, bool enable, const string & description = string())
    {
        auto parser = [name, enable, &destination](const string & value)
        {
            if (!value.empty())
                throw Redundant_Option_Value(name);

            destination = enable;
        };

        option_parsers.emplace(name, parser);
    }

    void parse(int argc, char * argv[])
    {
        // Skip executable name
        --argc;
        ++argv;

        int i = 0;
        while(i < argc)
        {
            string option = argv[i];

            string name;
            string value;

            auto separator_pos = option.find('=');

            name = option.substr(0,separator_pos);

            if (separator_pos != string::npos)
            {
                value = option.substr(separator_pos+1);
                if (value.empty())
                    throw Error("Invalid syntax: " + option);
            }

            auto parser_it = option_parsers.find(name);
            if (parser_it == option_parsers.end())
                throw Error("Invalid option: " + option);

            auto & parser = parser_it->second;
            parser(value);

            ++i;
        }
    }

private:
    unordered_map<string, Parser_Function> option_parsers;
};

}
