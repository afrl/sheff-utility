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

bool get_next_line(std::istream& in, std::string& str)
{
    getline(in, str, '\r');

    if (in.rdstate() == std::ios_base::badbit)
    {
        throw _UTIL_MSG_EXCEPT("An error occurred reading the next line: " + str);
    }

    // Note: "Notice that some eofbit cases will also set failbit."
    // Source: https://www.cplusplus.com/reference/string/string/getline/
    if (in.rdstate() == std::ios_base::eofbit || in.rdstate() == std::ios_base::failbit)
    {
        return true;
    }
    else
    {
        return false;
    }
}
}