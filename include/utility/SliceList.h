#pragma once

#include "Slice.h"

#include <array>
#include <type_traits>
#include <vector>

namespace utility
{
/// @brief Provides a view of data arranged as pointer to pointers.
///
/// Appropriate when data is arranged as a list of contiguous arrays in memory, with each contiguous
/// array being noncontiguous with the others, and there exists a contiguous array of pointers
/// referring to each array.
template<typename T, typename SliceSizeType = std::size_t, typename ListSizeType = std::size_t>
struct SliceList
{
    SliceList()
      : ptr(nullptr)
      , n_objects_list(nullptr)
      , n_arrays(0)
    {
    }
    SliceList(T* const * data, SliceSizeType* array_sizes, ListSizeType n_arrays)
      : ptr(data)
      , n_objects_list(array_sizes)
      , n_arrays(n_arrays)
    {
    }
    SliceList(const SliceList&)                         = default;
    SliceList<T, SliceSizeType, ListSizeType>& operator =(
      const SliceList<T, SliceSizeType, ListSizeType>&) = default;

    ListSizeType size() const
    {
        return n_arrays;
    }

    T** data()
    {
        return ptr;
    }

    /// Returns a non-const pointer to a const pointer of const objects
    const T* const* data() const
    {
        return ptr;
    }

    Slice<T, SliceSizeType> operator[](ListSizeType i)
    {
        return { ptr[i], n_objects_list[i] };
    }

    Slice<const T, const SliceSizeType> operator[](ListSizeType i) const
    {
        return { ptr[i], n_objects_list[i] };
    }

    /// Accessor that is redudant compared to operator[], but is needed for Python.
    Slice<const T, const SliceSizeType> get(ListSizeType i) const
    {
        return Slice<const T, const SliceSizeType>(ptr[i], n_objects_list[i]);
    }

    bool contains(const T& obj)
    {
        for (auto i : range(n_arrays))
        {
            for (auto j : range(n_objects_list[i]))
            {
                if (ptr[i][j] == obj)
                {
                    return true;
                }
            }
        }
        return false;
    }

    bool operator==(const SliceList& other) const
    {
        if (n_arrays != other.n_arrays)
        {
            return false;
        }
        // Check size of arrays matches
        for (auto i : range(n_arrays))
        {
            if (n_objects_list[i] != other.n_objects_list[i])
            {
                return false;
            }
        }

        bool comparison = true;
        for (auto i : range(n_arrays))
        {
            for (auto j : range(n_objects_list[i]))
            {
                comparison &= ptr[i][j] == other.ptr[i][j];
            }
        }
        return comparison;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Serailization methods

    template<typename SerializerType>
    void serialize(SerializerType& s) const
    {
        for (auto i : range(n_arrays))
        {
            s(utility::named_value(std::to_string(i), get(i)));
        }
    }

    template<typename SerializerType>
    void deserialize(SerializerType& s)
    {
        assert(ptr != nullptr);
        for (auto i : range(n_arrays))
        {
            s((*this)[i]);
        }
    }

  private:
    T* const * ptr;
    SliceSizeType* n_objects_list;
    ListSizeType n_arrays;
};

template<typename T, typename SliceSizeType, typename ListSizeType = std::size_t>
SliceList<T, SliceSizeType, ListSizeType> slice_list(T* const* const data,
                                                     SliceSizeType const* array_sizes,
                                                     const ListSizeType n_arrays)
{
    return { data, array_sizes, n_arrays };
}

template<typename T, typename SliceSizeType>
SliceList<const T, const SliceSizeType> slice_list(const std::vector<T*>& arrays,
                                                   const std::vector<SliceSizeType>& array_sizes)
{
    return { arrays.data(), array_sizes.data(), array_sizes.size() };
}

template<typename T, typename SliceSizeType>
SliceList<T, SliceSizeType> slice_list(std::vector<T*>& arrays,
                                       std::vector<SliceSizeType>& array_sizes)
{
    return SliceList<T, SliceSizeType> { arrays.data(), array_sizes.data(), array_sizes.size() };
}
}