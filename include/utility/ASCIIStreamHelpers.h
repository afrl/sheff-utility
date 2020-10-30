#pragma once

#include "utility/Exceptions.h"
#include "utility/StringUtils.h"

#include <assert.h>
#include <fstream>

namespace utility
{
/// Reads lines, starting with the current line if provided, until the line with the substring is
/// found.  If it is not found, an empty string will be returned.
/// @param substr - substring that if found within a line, iteration stops, and the line returned.
/// @param in - the input stream to read subsequent lines (and from which curr_line was read if
///        curr_line is provided).
/// @param curr_line (optional) - current line that will be checked for the substring, if it
///        contains the the substring, then no new lines will be read.
static std::string skip_to_line_that_contains(const std::string& substr,
                                              std::ifstream& in,
                                              std::string curr_line = "")
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
}