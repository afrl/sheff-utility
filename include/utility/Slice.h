#pragma once

#include <type_traits>
#include <vector>

namespace utility
{
/// Inspired by Rust, a slice refers to an array of a dynamic size in a contiguous
/// region of memory.
template<typename T>
struct Slice
{
    T* const data;
    const std::size_t size;

    T* begin() const
    {
        return data;
    }

    const T* end() const
    {
        return data + size;
    }

    Slice(T* begin_a, std::size_t size_a)
      : data(begin_a)
      , size(size_a)
    {
    }
};

template<typename T>
Slice<T> slice(T* const data, const std::size_t size)
{
    return { data, size };
}

template<typename T, typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
Slice<const T> slice(const std::vector<T>& vec)
{
    return Slice<const T> { vec.data(), vec.size() };
}

template<typename T, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
Slice<const typename std::underlying_type<T>::type> slice(const std::vector<T>& vec)
{
    using under_type = typename std::underlying_type<T>::type;
    return Slice<const under_type> { reinterpret_cast<const under_type*>(vec.data()),
                            vec.size() };
}

template<typename T, int DIM>
Slice<T> slice(const std::array<T, DIM>& arr)
{
    return { arr.data(), arr.size() };
}
}