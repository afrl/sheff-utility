#pragma once

#include "Exceptions.h"
#include "Range.h"

#include <array>
#include <type_traits>
#include <vector>

namespace utility
{
/// Inspired by Rust, a slice refers to an array of a dynamic size in a contiguous
/// region of memory.
template<typename T, typename SizeType = std::size_t>
struct Slice
{
    Slice()
      : ptr(nullptr)
      , n_objects(0)
    {
    }
    Slice(T* begin_a, SizeType n_objects_a)
      : ptr(begin_a)
      , n_objects(n_objects_a)
    {
    }
    Slice(const Slice<T, SizeType>&) = default;
    Slice<T, SizeType>& operator=(const Slice<T, SizeType>&) = default;

    T* begin() const
    {
        return ptr;
    }

    T* end() const
    {
        return ptr + n_objects;
    }

    SizeType size() const
    {
        return n_objects;
    }

    T& front()
    {
        _UTIL_ASSERT_C(ptr != nullptr, "The underlying data for the slice is null.");
        return ptr[0];
    }

    const T& front() const
    {
        _UTIL_ASSERT_C(ptr != nullptr, "The underlying data for the slice is null.");
        return ptr[0];
    }

    T& back()
    {
        _UTIL_ASSERT_C(ptr != nullptr, "The underlying data for the slice is null.");
        return ptr[n_objects - 1];
    }

    const T& back() const
    {
        _UTIL_ASSERT_C(ptr != nullptr, "The underlying data for the slice is null.");
        return ptr[n_objects - 1];
    }

    T* data()
    {
        return ptr;
    }

    const T* data() const
    {
        return ptr;
    }

    T& operator[](SizeType i)
    {
        return ptr[i];
    }

    const T& operator[](SizeType i) const
    {
        return ptr[i];
    }

    /// Accessor that is redudant compared to operator[], but is needed for Python.
    const T& get(SizeType i) const
    {
        _UTIL_ASSERT_C(ptr != nullptr, "The underlying data for the slice is null.");
        return ptr[i];
    }

    bool contains(const T& obj)
    {
        for (auto i : utility::range(n_objects))
        {
            if (ptr[i] == obj)
            {
                return true;
            }
        }
        return false;
    }

    bool operator==(const Slice<T, SizeType>& other) const
    {
        if (n_objects != other.n_objects)
        {
            return false;
        }
        bool comparison = true;
        for (auto i : utility::range(n_objects))
        {
            comparison &= ptr[i] == other.ptr[i];
        }
        return comparison;
    }

  private:
    T* ptr;
    SizeType n_objects;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// Slice creators for common data types (mutable)

template<typename T, typename SizeType = std::size_t>
Slice<T, SizeType> slice(T* const data, const SizeType size)
{
    return { data, size };
}

template<typename T, typename SizeType = std::size_t>
Slice<T, SizeType> slice(T* const data, const SizeType rows, const SizeType cols)
{
    return { data, rows * cols };
}

template<typename T,
         typename SizeType                                          = std::size_t,
         typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
Slice<T, SizeType> slice(std::vector<T>& vec)
{
    return Slice<T, SizeType> { vec.data(), static_cast<SizeType>(vec.size()) };
}

template<typename T,
         typename SizeType                                         = std::size_t,
         typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
Slice<typename std::underlying_type<T>::type, SizeType> slice(std::vector<T>& vec)
{
    using under_type = typename std::underlying_type<T>::type;
    return Slice<under_type, SizeType> { reinterpret_cast<under_type*>(vec.data()),
                                         static_cast<SizeType>(vec.size()) };
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Slice creators for common data types (const)

template<typename T, typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
Slice<const T> slice_c(const std::vector<T>& vec)
{
    return Slice<const T> { vec.data(), vec.size() };
}

template<typename T, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
Slice<const typename std::underlying_type<T>::type> slice_c(const std::vector<T>& vec)
{
    using under_type = typename std::underlying_type<T>::type;
    return Slice<const under_type> { reinterpret_cast<const under_type*>(vec.data()), vec.size() };
}

template<typename T, int DIM>
Slice<const T> slice_c(const std::array<T, DIM>& arr)
{
    return Slice<T>(arr.data(), arr.size());
}
}
