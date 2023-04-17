#pragma once

#include <algorithm>
#include <vector>

namespace utility
{

// @brief Erase a list of indices from a vector of data.
// TODO: make this generic to work for DynamicArray 
// 
// Credit:
// https://stackoverflow.com/questions/7571937/how-to-delete-items-from-a-stdvector-given-a-list-of-indices
template<typename T, typename IndexType>
inline void erase_by_indices(std::vector<T>& data, std::vector<IndexType>& indices)
{
    if (indices.empty())
    {
        return;
    }

    std::vector<T> ret;
    ret.reserve(data.size() - indices.size());

    std::sort(indices.begin(), indices.end());

    // new we can assume there is at least 1 element to delete. copy blocks at a time.
    std::vector<T>::iterator curr_block_begin_iter = data.begin();
    for (std::vector<IndexType>::iterator it = indices.begin(); it != indices.end(); ++it)
    {
        std::vector<T>::iterator curr_block_end_iter = data.begin() + *it;
        if (curr_block_begin_iter != curr_block_end_iter)
        {
            std::copy(curr_block_begin_iter, curr_block_end_iter, std::back_inserter(ret));
        }
        curr_block_begin_iter = curr_block_end_iter + 1;
    }

    // copy last block.
    if (curr_block_begin_iter != data.end())
    {
        std::copy(curr_block_begin_iter, data.end(), std::back_inserter(ret));
    }

    ret.swap(data);
}
}