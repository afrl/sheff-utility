#include <string>

namespace utility
{
	/// Transforms a string to upper case
	void str_to_upper(std::string& s);
	/// Transforms a string to lower case
	void str_to_lower(std::string& s);
	/// Performs a case-insensitive string comparison
	bool istr_is_equal(const std::string &s1, const std::string &s2);
	/// Determines if a string is a number
	bool is_number(const std::string& s);
	/// Converts a string to an unsigned integer
	unsigned int str_to_uint(const std::string &s);
	/// Converts a string that could be x, y, z, 0, 1, or 2
	/// to the 0-2 index for a coordinate axis
	unsigned int str_to_axis(const std::string &s);
}