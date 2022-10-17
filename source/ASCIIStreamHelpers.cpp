#include "utility/ASCIIStreamHelpers.h"

#include "utility/Exceptions.h"
#include "utility/StringUtils.h"

#include <assert.h>
#include <iostream>

namespace utility
{
std::string skip_to_line_that_contains(const std::string& substr,
                                       std::ifstream& in,
                                       std::string curr_line)
{
    if (!in.is_open())
    {
        throw _UTIL_MSG_EXCEPT("File stream not open.");
    }
    assert(substr != "");

    // The current line already contains the substring
    if (curr_line.find(substr) != std::string::npos)
    {
        return curr_line;
    }

    std::string line = "";
    do
    {
        getline(in, line);
        if (in.eof())
        {
            return "";
        }
    } while (line.find(substr) == std::string::npos && !in.eof());
    return line;
}

std::istream& get_next_line(std::istream& in, std::string& str)
{
    // Taken from:
    // https://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf

    str.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(in, true);
    auto sb = in.rdbuf();

    for (;;)
    {
        int c = sb->sbumpc();
        switch (c)
        {
            case '\n':
                if (!str.empty())
                {
                    return in;
                }
                break;
            case '\r':
                if (sb->sgetc() == '\n')
                {
                    sb->sbumpc();
                }
                if (!str.empty())
                {
                    return in;
                }
                break;
            case EOF:
                // Also handle the case when the last line has no line ending
                if (str.empty())
                {
                    in.setstate(std::ios::eofbit);
                    return in;
                }
                if (!str.empty())
                {
                    return in;
                }
                break;
            default: str += (char)c;
        }
    }
}
}