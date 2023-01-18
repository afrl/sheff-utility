#pragma once

#include "DynamicArray.h"
#include "Exceptions.h"
#include "Range.h"

#include <bitset>
#include <utility>
#include <vector>

namespace utility
{
/// @brief Provides a dynamic bitset by building on top of std::bitset.
/// This class stores an array of std::bitset objects, each with size BLOCK_SIZE.  There exists a
/// trade off between the block size and the number of blocks.  If you expect the bitset at runtime
/// to typically have a certain size, consider trying to align the block size with it to minimize
/// wasted space.  The block size optimally should be a multiple of 8 (up to 64).
/// @attention The maximum size of the bitset is 256 * BLOCK_SIZE.
/// @attention This works well for relatively dense bitsets (greater than 1/8th of the size testing
///            true.  For a more sparse bitset, consider the Roaring library.  And for static
///            bitset's consider using std::bitset directly.
template<uint8_t BLOCK_SIZE>
class DynamicBitset
{
  public:
    using BlockSizeType = uint8_t;
    using SizeType      = uint64_t;

    static_assert(BLOCK_SIZE <= 64, "DynamicBitset requires a block size of 64 or less");

    DynamicBitset()                         = default;
    DynamicBitset(const DynamicBitset&)     = default;
    DynamicBitset(DynamicBitset&&) noexcept = default;
    /// Allocates the bitset to size n_bits
    DynamicBitset(const SizeType n_bits)
      : DynamicBitset()
    {
        resize(n_bits);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    DynamicBitset& operator=(const DynamicBitset&) = default;
    DynamicBitset& operator=(DynamicBitset&&) noexcept = default;

    /// Resizes to the next largest multiple of BLOCK_SIZE
    inline void resize(const SizeType n_bits)
    {
        _UTIL_ASSERT(n_bits < (uint64_t)256 * BLOCK_SIZE,
                     "Requested bitset size is greater than the maximum allowable size.");
        BlockSizeType n_blocks =
          static_cast<BlockSizeType>(n_bits / BLOCK_SIZE + (n_bits % BLOCK_SIZE != 0));
        bitset_blocks_.resize(n_blocks);
    }
    inline DynamicBitset& flip(const SizeType& i)
    {
        const auto b = get(i);
        b.first.flip(b.second);
        return *this;
    }
    inline DynamicBitset& set(const SizeType& i, const bool& v)
    {
        const auto b = get(i);
        b.first.set(b.second, v);
        return *this;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Accessors

    /// Returns the value of the bitset at position i
    inline bool test(const SizeType& i) const
    {
        const auto b = get(i);
        return b.first.test(b.second);
    }

    /// Returns the size of the dynamic bitset (how many bits can fit in it)
    /// @attention The size is not neccessarily what was given to the constructor or resize.  It is
    /// the next largest multiple of BLOCK_SIZE.
    inline SizeType size() const
    {
        return bitset_blocks_.size() * BLOCK_SIZE;
    }

    /// Returns the number of true bits in the set
    inline SizeType count() const
    {
        SizeType c = 0;
        for (auto& block : bitset_blocks_)
        {
            c += block.count();
        }
        return c;
    }

    inline bool empty() const
    {
        bool empty = true;
        for (auto& block : bitset_blocks_)
        {
            empty &= block.count() == 0;
        }
        return empty;
    }

    inline std::vector<unsigned long long> to_ullongs() const
    {
        std::vector<unsigned long long> block_integer_repr;
        for (auto& bitset : bitset_blocks_)
        {
            block_integer_repr.push_back(bitset.to_ullong());
        }
        return block_integer_repr;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////
    // Comparators

    inline bool operator==(const DynamicBitset& other) const
    {
        return bitset_blocks_ == other.bitset_blocks_;
    }
    inline bool operator!=(const DynamicBitset& other) const
    {
        return !(bitset_blocks_ == other.bitset_blocks_);
    }

    /// @brief Returns true if the integer presentation of each blocks is less than the
    ///        corresponding block in the other DynamicBitset.
    /// @attention This may be useful for nothing except map data structures.
    inline bool operator<(const DynamicBitset& other) const
    {
        _UTIL_ASSERT(
          size() == other.size() && block_size() == other.block_size(),
          "The bitsets must be the same size and have the same block size for the < operator");

        for (auto i = bitset_blocks_.size() - 1; i >= 0; i--)
        {
            if (bitset_blocks_[i].to_ullong() < other.bitset_blocks_[i].to_ullong())
            {
                return true;
            }
            else if (bitset_blocks_[i].to_ullong() > other.bitset_blocks_[i].to_ullong())
            {
                return false;
            }
            // Loop continues if the blocks were equal
        }
        // All blocks are equal
        return false;
    }

  private:
    DynamicArray<std::bitset<BLOCK_SIZE>, uint8_t> bitset_blocks_;

    inline BlockSizeType block_size() const
    {
        return BLOCK_SIZE;
    }

    /// Returns the bitset corresponding to the block containing i and the respective index
    /// within the block
    inline std::pair<std::bitset<BLOCK_SIZE>&, BlockSizeType> get(const SizeType& i)
    {
        const auto block_index = static_cast<BlockSizeType>(i / BLOCK_SIZE);
        _UTIL_ASSERT_C(i / BLOCK_SIZE < bitset_blocks_.size(), "Exceeds # of blocks.");
        auto& block      = bitset_blocks_[block_index];
        const auto index = static_cast<BlockSizeType>(i % BLOCK_SIZE);
        _UTIL_ASSERT_C(index < block.size(), "Exceeds size in block.");
        return { block, index };
    }
    inline std::pair<const std::bitset<BLOCK_SIZE>&, BlockSizeType> get(const SizeType& i) const
    {
        const auto block_index = i / BLOCK_SIZE;
        _UTIL_ASSERT_C(i / BLOCK_SIZE < bitset_blocks_.size(), "Exceeds # of blocks.");
        auto& block      = bitset_blocks_[block_index];
        const auto index = i % BLOCK_SIZE;
        _UTIL_ASSERT_C(index < block.size(), "Exceeds size in block.");
        return { block, index };
    }
};
}