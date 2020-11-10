#pragma once

#include <algorithm>  // std::stable_sort
#include <functional> // std::hash
#include <numeric>    // std::iota
#include <utility>    // std::pair

namespace utility
{
/// Creates a combined hash for a homogeneous std::pair.
template<typename T>
std::size_t hash(const std::pair<T, T>& x)
{
    return hash(x.first, x.second);
}

/// Creates a combined hash for two objects of the same type.
template<typename T>
std::size_t hash(const T& x, const T& y)
{
    auto hash_x = std::hash<T>()(x);
    return std::hash<T>()(y) + 0x9e3779b9 + (hash_x << 6) + (hash_x >> 2);
}

/// Creates a combined hash for a list of objects in a container.
template<typename ContainerType>
std::size_t hash(const ContainerType& vec)
{
    std::size_t seed = vec.size();
    for (auto& i : vec)
    {
        seed ^= std::hash<std::size_t>()((std::size_t)i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

/// Creates a combined hash for a homogeneous std::pair as if they were sorted, so the hash is
/// insentive to order.
template<typename T>
std::size_t hash_sorted(const std::pair<T, T>& x)
{
    return hash_sorted(x.first, x.second);
}

/// Creates a combined hash for two objects of the same type as if they were sorted, so the hash is
/// insentive to order.
template<typename T>
std::size_t hash_sorted(const T& x, const T& y)
{
    if (x < y)
    {
        return utility::hash(x, y);
    }
    else
    {
        return utility::hash(y, x);
    }
}

/// Creates a combined hash for a list of objects in a container as if it were sorted, so the hash
/// is insentive to order.
template<typename ContainerType>
std::size_t hash_sorted(const ContainerType& vec)
{
    std::vector<std::size_t> hashes;
    hashes.reserve(vec.size());
    for (auto& obj : vec)
    {
        hashes.push_back(std::hash<std::size_t>()((std::size_t)obj));
    }

    std::vector<std::size_t> sorted_indices(hashes.size());
    std::iota(sorted_indices.begin(), sorted_indices.end(), 0);
    std::stable_sort(sorted_indices.begin(), sorted_indices.end(), [&vec](size_t i1, size_t i2) {
        return vec[i1] < vec[i2];
    });

    std::size_t seed = vec.size();
    for (auto& i : sorted_indices)
    {
        seed ^= hashes[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

}
