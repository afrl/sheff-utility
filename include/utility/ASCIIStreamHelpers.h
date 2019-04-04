#pragma once

#include "StringUtils.h"

#include <fstream>

namespace utility
{
    /// Reads lines until the line with the substring is found.  If it is not found,
    /// an empty string will be returned.
    static std::string skip_to_line_that_contains(const std::string & substr, std::ifstream & in)
    {
        if (!in.is_open())
        {
            throw _UTIL_MSG_EXCEPT("File stream not open.");
        }

        std::string line = "";
        do
        {
            getline(in, line);
            if (in.eof())
            {
                throw _UTIL_MSG_EXCEPT("Substring '" + substr + "' not found in file.");
            }
        } while (line.find(substr) == std::string::npos && !in.eof());
        return line;
    }
}