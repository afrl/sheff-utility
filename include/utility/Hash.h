#pragma once

#include <functional>
#include <utility>

namespace utility
{
    struct hash {
    public:
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T, U>& x) const
        {
            return utility::hash()(x.first, x.second);
        }

        template <typename T, typename U>
        std::size_t operator()(const T& x, const U& y) const
        {
            return std::hash<T>()(x) ^ std::hash<U>()(y);
        }
    };

    // Theoretically, the XOR order shouldn't matter, but we can ensure consistency by 
    // sorting first. 
    // NOTE: This will cause an error if the types you want to compare aren't compatible.
    struct hash_sorted {
    public:
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T, U>& x) const
        {
            return utility::hash_sorted()(x.first, x.second);
        }

        template <typename T, typename U>
        std::size_t operator()(const T& x, const U& y) const
        {
            if (x < y)
            {
                return std::hash<T>()(x) ^ std::hash<U>()(y);
            }
            else
            {
                return std::hash<U>()(y) ^ std::hash<T>()(x);
            }
        }
    };

}
