#include "utility/StringUtils.h"

#include <algorithm>
#include <sstream>
#include <iterator>

#if defined(_WIN32) || defined(_WIN64)
#define _ISTR_COMPARE _stricmp
#include <string.h>
#else
#define _ISTR_COMPARE strcasecmp
#include <strings.h>
#endif

namespace utility
{

    void str_to_upper(std::string & s)
    {
        std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))::toupper);
    }

    void str_to_lower(std::string & s)
    {
        std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))::tolower);
    }

    bool istr_is_equal(const std::string & s1, const std::string & s2)
    {
        return _ISTR_COMPARE(s1.c_str(), s2.c_str()) == 0;
    }

    bool is_number(const std::string& s)
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && ::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    }

    std::vector<std::string> tokenize(const std::string & s)
    {
        std::istringstream ss(s);
        std::istream_iterator<std::string> current(ss), end;
        
        std::vector<std::string> tokens;
        tokens.insert(tokens.begin(), current, end);
        return tokens;
    }
    
    void remove_substring(std::string & s, const std::string & substr)
    {
        size_t pos = std::string::npos;
 
        while ((pos  = s.find(substr)) != std::string::npos)
        {
            // If found then erase it from string
            s.erase(pos, substr.length());
        }
    }
}