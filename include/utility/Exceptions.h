#pragma once

#include <iostream>
#include <string>
#include <exception>
#include <typeinfo>

#ifdef __GNUG__
#   include <cstdlib>
#   include <memory>
#   include <cxxabi.h>
#endif

namespace utility
{
    static std::string demangle( const char* name )
    {
#ifdef __GNUG__
        // From here:
        // https://stackoverflow.com/questions/281818/unmangling-the-result-of-stdtype-infoname

        int status = -4; // some arbitrary value to eliminate the compiler warning

        // enable c++11 by passing the flag -std=c++11 to g++
        std::unique_ptr<char, void(*)(void*)> res {
            abi::__cxa_demangle(name, NULL, NULL, &status),
                std::free
                };
        return ( status == 0 ) ? res.get() : name;
#else
        return name;
#endif
    }
} // end namespace utility

// Use this in functions
#define _UTIL_MSG_EXCEPT( msg )   utility::MsgException( __FILE__, "", __func__, __LINE__, msg )

// Use this in class methods
#define _UTIL_MSG_EXCEPT_C( msg ) utility::MsgException( __FILE__, demangle( typeid(*this).name() ), __func__, __LINE__, msg )

// Assertion for functions
#ifdef DISABLE_ASSERTIONS
#    define _UTIL_ASSERT(x, m)
#else
#    define _UTIL_ASSERT(x, m) if (!(x)) { throw _UTIL_MSG_EXCEPT(m); }
#endif

// Assertion for class methods
#ifdef DISABLE_ASSERTIONS
#    define _UTIL_ASSERT_C(x, m)
#else
#    define _UTIL_ASSERT_C(x, m) if (!(x)) { throw _UTIL_MSG_EXCEPT_C(m); }
#endif

#define _UTIL_WARN(m) std::cerr << utility::build_warning_msg(__FILE__, "", __func__, __LINE__, m) << std::endl;
#define _UTIL_WARN_C(m) std::cerr << utility::build_warning_msg(__FILE__, typeid(*this).name(), __func__, __LINE__, m) << std::endl;

namespace utility
{
    static std::string build_warning_msg(const std::string file, const std::string cls, const std::string func, const int line, const std::string msg = "")
    {
        return "Warning " + msg + "\n\t\tOcurred in:\n\t\t" + file + ":" + std::to_string(line) + " in\n\t\t" +
               ((cls == "") ? " function " + func : func + " method of " + cls);
    }

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
            msg = "An error occurred in:\n\t\tFile:     " + file + ":" + std::to_string(line) + "\n\t\t" +
                ((cls == "") ? "Function: " + func : "Method:   "  + func + "() from Class " + cls ) +
                ((addMsg != "") ? "\n\t\tMessage:  " + addMsg : ".\n");
        }

        virtual const char* what() const noexcept { return msg.c_str(); }

    private:
        std::string msg;
    };

} // end namespace utility
