#pragma once

#include "UniquePtrForwardIteratorWrapper.h"
#include "serialization/NamedValue.h"

#include <array>
#include <memory>
#include <type_traits>
#include <vector>

namespace utility
{
/// Similar to the Slice class, but allows [] operator to access the objects inside a unique_ptr.
template<typename T, typename PtrType>
struct UniquePtrSlice
{
    UniquePtrSlice(PtrType* begin_a, std::size_t n_objects_a)
      : ptr(begin_a)
      , n_objects(n_objects_a)
    {
    }

    PtrType* begin() const
    {
        return ptr;
    }

    const PtrType* end() const
    {
        return ptr + n_objects;
    }

    std::size_t size() const
    {
        return n_objects;
    }

    std::unique_ptr<T>* data()
    {
        return ptr;
    }

    const std::unique_ptr<T>* data() const
    {
        return ptr;
    }

    T& operator[](std::size_t i)
    {
        return *ptr[i];
    }

    const T& operator[](std::size_t i) const
    {
        return *ptr[i];
    }


    template<typename SerializerType>
    void serialize(SerializerType& s) const
    {
        for (auto i = 0; i < n_objects; i++)
        {
            s(utility::named_ref(std::to_string(i), *ptr[i]));
        }
    }

  private:
    PtrType* const ptr;
    const std::size_t n_objects;
};

template<typename T>
UniquePtrSlice<T, const std::unique_ptr<T>> unique_ptr_slice(const std::unique_ptr<T>* const data,
                                                             const std::size_t size)
{
    return { data, size };
}

template<typename T>
UniquePtrSlice<T, std::unique_ptr<T>> unique_ptr_slice(std::unique_ptr<T>* const data,
                                                       const std::size_t size)
{
    return { data, size };
}
}
