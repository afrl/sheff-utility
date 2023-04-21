#pragma once

#include <vector>

namespace utility
{
/// @brief Given a vector and a function to compare items, returns the backwards map of indices.
/// @param v - vector to find the permution that would sort it
/// @param compare - comparison function that takes two items (A and B) and returns A < B
/// @return a vector filled with the old indices in the order that should
///         form the new vector, which is effectively a map from the new to the old indices
template<typename T, typename Compare>
std::vector<std::size_t> find_sorted_permutation(const std::vector<T>& v, const Compare& comp)
{
    std::vector<std::size_t> backward_map(v.size());
    std::iota(backward_map.begin(), backward_map.end(), 0);
    std::sort(backward_map.begin(),
              backward_map.end(),
              [&](const std::size_t& i, const std::size_t& j) { return comp(v[i], v[j]); });
    return backward_map;
}

/// @brief Given a vector, returns the backwards map of indices (based on intrisic < operator)
/// @param v - vector to find the permution that would sort it
/// @return a vector filled with the old indices in the order that should
///         form the new vector, which is effectively a map from the new to the old indices
template<typename T>
std::vector<std::size_t> find_sorted_permutation(const std::vector<T>& v)
{
    std::vector<std::size_t> backward_map(v.size());
    std::iota(backward_map.begin(), backward_map.end(), 0);
    std::sort(backward_map.begin(),
              backward_map.end(),
              [&](const std::size_t& i, const std::size_t& j) { return v[i] < v[j]; });
    return backward_map;
}

/// Helper function to permute a vector:
/// https://devblogs.microsoft.com/oldnewthing/20170102-00/?p=95095
/// @param v - vector to permute (in/out)
/// @param backward_map - a vector filled with the old indices in the order that should
///        form the new vector, which is effectively a map from the new to the old indices
template<typename T, typename IndexType>
void apply_permutation(std::vector<T>& v, const std::vector<IndexType>& backward_map)
{
    std::vector<T> permuted_v;
    permuted_v.reserve(backward_map.size());
    for (IndexType i = 0; i < backward_map.size(); i++)
    {
        permuted_v.push_back(std::move(v[backward_map[i]]));
    }
    v = std::move(permuted_v);
}

template<typename IndexType>
std::vector<IndexType> get_inverse_map(const std::vector<IndexType>& map)
{
    std::vector<IndexType> inverse_map(map.size(), -1);
    for (auto i : utility::indices(map))
    {
        inverse_map[map[i]] = static_cast<IndexType>(i);
    }
    assert(std::none_of(inverse_map.begin(),
                        inverse_map.end(),
                        [](const IndexType& i) -> bool { return i == -1; }));
    return inverse_map;
}
}