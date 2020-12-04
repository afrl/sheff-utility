#pragma once

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
    Slice(const Slice&) = default;
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

    bool operator==(const Slice<T>& other) const
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

template<typename T, typename SizeType = std::size_t>
Slice<T, SizeType> slice(T* const data, const SizeType size)
{
    return { data, size };
}

template<typename T, typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
Slice<const T> slice(const std::vector<T>& vec)
{
    return Slice<const T> { vec.data(), vec.size() };
}

template<typename T, typename std::enable_if<!std::is_enum<T>::value, T>::type* = nullptr>
Slice<T> slice(std::vector<T>& vec)
{
    return Slice<T> { vec.data(), vec.size() };
}

template<typename T, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
Slice<const typename std::underlying_type<T>::type> slice(const std::vector<T>& vec)
{
    using under_type = typename std::underlying_type<T>::type;
    return Slice<const under_type> { reinterpret_cast<const under_type*>(vec.data()), vec.size() };
}

template<typename T, typename std::enable_if<std::is_enum<T>::value, T>::type* = nullptr>
Slice<typename std::underlying_type<T>::type> slice(std::vector<T>& vec)
{
    using under_type = typename std::underlying_type<T>::type;
    return Slice<under_type> { reinterpret_cast<under_type*>(vec.data()), vec.size() };
}

template<typename T, int DIM>
Slice<T> slice(const std::array<T, DIM>& arr)
{
    return { arr.data(), arr.size() };
}
}