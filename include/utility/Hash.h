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
            return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
        }

        template <typename T, typename U>
        std::size_t operator()(const T & x, const U & y) const
        {
            return std::hash<T>()(x) ^ std::hash<U>()(y);
        }
    };

    struct hash_sorted {
    public:
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T, U>& x) const
        {
            return utility::hash( x.first, x.second );
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
                return std::hash<T>()(y) ^ std::hash<U>()(x);
            }
        }
    };
}
