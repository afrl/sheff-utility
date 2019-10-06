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

        /// The earlier implementation of the hash combination using the XOR
        /// operator was too weak (really weak).  Collisions were common.  This
        /// version of the hash combine will be order sensitive, and will greatly
        /// reduce collisions.  In fact, the hash should be order sensitive, so
        /// this fixes that bug as well as collitions of completely different
        /// combinations.
        template <typename T, typename U>
        std::size_t operator()(const T& x, const U& y) const
        {
            auto hash_x = std::hash<T>()(x);
            return std::hash<T>()(y) + 0x9e3779b9 + (hash_x << 6) + (hash_x >> 2);
        }
    };

    struct hash_sorted {
    public:
        template <typename T, typename U>
        std::size_t operator()(const std::pair<T, U>& x) const
        {
            return utility::hash(x.first, x.second);
        }

        template <typename T, typename U>
        std::size_t operator()(const T& x, const U& y) const
        {
            if (x < y)
            {
                return utility::hash()(x, y);
            }
            else
            {
                return utility::hash()(y, x);
            }
        }
    };

}
