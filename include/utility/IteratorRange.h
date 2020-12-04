#pragma once

namespace utility
{
/// @brief A template class that holds a begin and end iterator to describe a range and use with
/// ranged-based loops.
template<typename IteratorType>
struct IteratorRange
{
    IteratorType begin()
    {
        return begin_;
    }
    const IteratorType end() const
    {
        return end_;
    }

    IteratorType begin_;
    const IteratorType end_;
};
}