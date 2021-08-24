#pragma once

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
std::string skip_to_line_that_contains(const std::string& substr,
                                       std::ifstream& in,
                                       std::string curr_line = "");

/// @brief Equivalent functionality to getline but correctly delimits both Windows and Linux line
/// endings regardless of the system and throws on error.
/// 
/// @returns Bool indicating if EOF is reached.
bool get_next_line(std::istream& in, std::string& str);
}