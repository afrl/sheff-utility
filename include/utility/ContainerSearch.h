#pragma once

#include <utility>

namespace utility
{
    template<typename T>
    static std::pair<typename T::value_type, typename T::Index> find_max(const T & container)
    {
        typename T::Index max_index = 0;
        typename T::value_type max_value = *container.begin();

        typename T::Index curr_index = 0;
        for (auto value : container)
        {
            if (value > max_value)
            {
                max_index = curr_index;
            }
            curr_index++;
        }

        return { max_value, max_index };
    }
}