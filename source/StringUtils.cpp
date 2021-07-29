#include "..\include\utility\StringUtils.h"

#include "utility/StringUtils.h"

#include <algorithm>
#include <iterator>
#include <sstream>

#if defined(_WIN32) || defined(_WIN64)
#define _ISTR_COMPARE _stricmp
#include <string.h>
#else
#define _ISTR_COMPARE strcasecmp
#include <strings.h>
#endif

namespace utility
{

void str_to_upper(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))::toupper);
}

void str_to_lower(std::string& s)
{
    std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))::tolower);
}

std::string ltrim(const std::string& s)
{
    std::string t = s;
    t.erase(t.begin(), find_if_not(t.begin(), t.end(), [](int c) { return isspace(c); }));
    return t;
}

std::string rtrim(const std::string& s)
{
    std::string t = s;
    t.erase(find_if_not(t.rbegin(), t.rend(), [](int c) { return isspace(c); }).base(), t.end());
    return t;
}

std::string trim(const std::string& s)
{
    return ltrim(rtrim(s));
}


bool istr_is_equal(const std::string& s1, const std::string& s2)
{
    return _ISTR_COMPARE(s1.c_str(), s2.c_str()) == 0;
}

bool is_number(const std::string& s)
{
    char* p;
    strtod(s.c_str(), &p);
    return *p == 0;
}

unsigned int str_to_axis(const std::string& s)
{
    if (istr_is_equal(s, "x"))
    {
        return 0;
    }
    else if (istr_is_equal(s, "y"))
    {
        return 1;
    }
    else if (istr_is_equal(s, "z"))
    {
        return 2;
    }
    else
    {
        return std::atoi(s.c_str());
    }
}

std::vector<std::string> tokenize(const std::string& s)
{
    std::istringstream ss(s);
    std::istream_iterator<std::string> current(ss), end;

    std::vector<std::string> tokens;
    tokens.insert(tokens.begin(), current, end);
    return tokens;
}

void remove_substring(std::string& s, const std::string& substr)
{
    size_t pos = std::string::npos;

    while ((pos = s.find(substr)) != std::string::npos)
    {
        // If found then erase it from string
        s.erase(pos, substr.length());
    }
}

void replace_substring(std::string& s,
                       const std::string& to_replace,
                       const std::string& replacement)
{
    if (to_replace.empty())
    {
        return;
    }

    std::size_t start_pos = 0;
    while ((start_pos = s.find(to_replace, start_pos)) != std::string::npos)
    {
        s.replace(start_pos, to_replace.length(), replacement);
        start_pos += replacement.length();
    }
}
}
