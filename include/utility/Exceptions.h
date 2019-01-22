#pragma once

#include <string>
#include <exception>
#include <typeinfo>

// Use this in functions
#define _UTIL_MSG_EXCEPT(m) utility::MsgException(__FILE__, "", __func__, __LINE__, m)
// Use this in class methods
#define _UTIL_MSG_EXCEPT_C(m) utility::MsgException(__FILE__, typeid(*this).name(), __func__, __LINE__, m)
// Assertion for functions
#ifdef DISABLE_ASSERTIONS
#define _UTIL_ASSERT(x, m)
#else
#define _UTIL_ASSERT(x, m) if (x) { _UTIL_MSG_EXCEPT(m); }
#endif
// Assertion for class methods
#ifdef DISABLE_ASSERTIONS
#define _UTIL_ASSERT_C(x, m)
#else
#define _UTIL_ASSERT_C(x, m) if (x) { _UTIL_MSG_EXCEPT_C(m); }
#endif

namespace utility
{
	/// \brief A simple exception class to throw inside Beta.
	/// Use the macro's defined above in this header file.  The uses are as follows:
	/// - _UTIL_MSG_EXCEPT(string message) should be used in functions
	/// - _UTIL_MSG_EXCEPT_C(string message) should be used in classes
	/// For example:
	/// \code{.cpp}
	/// void foo() {
	///     throw _UTIL_MSG_EXCEPT("Not implemented.");
	/// }
	/// \endcode
	class MsgException : public std::exception
	{
	public:
		MsgException(const std::string file, const std::string cls, const std::string func, const int line, const std::string addMsg = "") {
			msg = "An error occured in \n\t\t" + file + ":" + std::to_string(line) + " in\n\t\t" + ((cls == "") ? " function " + func : func + " method of " + cls)
				+ ((addMsg != "") ? ". With message:\n\n" + addMsg : ".\n");
		}

		virtual const char* what() const noexcept {
			return msg.c_str();
		}

	private:
		std::string msg;
	};
}