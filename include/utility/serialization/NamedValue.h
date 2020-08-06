#pragma once

#include <string>

namespace utility
{
/// @brief Simple struct to hold a name and reference to a value for serialization to JSON.
template<typename T>
struct NamedValue
{
    std::string name;
    T value;
};

/// Create a NamedValue object from a name and reference.  Note: the NamedValue keeps a reference to
/// value for its lifetime.
template<typename T>
NamedValue<T&> named_ref(std::string name, T& value)
{
    return NamedValue<T&>({ name, value });
}

/// Create a NamedValue object from a name and reference.  Note: the NamedValue keeps a reference to
/// value for its lifetime.
template<typename T>
NamedValue<T> named_value(std::string name, T value)
{
    return NamedValue<T>({ name, value });
}
}