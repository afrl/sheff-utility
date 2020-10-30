#pragma once

#include <type_traits>
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
template<typename T, typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
NamedValue<T&> named_ref(std::string name, T& value)
{
    return NamedValue<T&>({ name, value });
}

/// Create a NamedValue object from a name and reference.  Note: the NamedValue keeps a reference to
/// value for its lifetime.
template<typename T, typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
NamedValue<T> named_value(std::string name, T value)
{
    return NamedValue<T>({ name, value });
}


/// enum class named_value(...) specialization
template<typename T, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
NamedValue<typename std::underlying_type<T>::type> named_value(std::string name, T enum_value)
{
    using under_type = typename std::underlying_type<T>::type;
    return NamedValue<under_type>({ name, static_cast<under_type>(enum_value) });
}

}