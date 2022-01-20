/// Unit test for DynamicBitset.h

#include "utility/DynamicBitset.h"
#include "utility/Range.h"

#include <assert.h>
#include <cinttypes>
#include <iostream>
#include <set>

void set_from_int(int64_t n, utility::DynamicBitset<8>& bitset)
{

    int64_t i = 0;
    while (n > 0)
    {
        bitset.set(i, n % 2 == 1);
        n = n / 2;
        i++;
    }
}

int main()
{
    utility::DynamicBitset<8> small(4);
    small.flip(0);
    small.flip(3);
    assert(!small.empty());
    assert(small.size() == 8);
    assert(small.test(0));
    assert(!small.test(1));
    assert(!small.test(2));
    assert(small.test(3));
    _UTIL_ASSERT_THROW(small.flip(-1));
    // Note: small allows up to 8 bits even though it was resized to 4 since the block size is 8
    _UTIL_ASSERT_THROW(small.flip(9));

    utility::DynamicBitset<8> medium(16);
    medium.flip(0);
    medium.flip(3);
    medium.flip(10);
    medium.flip(15);
    assert(medium.size() == 16);
    assert(!medium.empty());
    assert(medium.test(0));
    assert(medium.test(3));
    assert(medium.test(10));
    assert(medium.test(15));
    _UTIL_ASSERT_THROW(medium.flip(16));
    _UTIL_ASSERT_THROW(medium.flip(17));

    utility::DynamicBitset<16> medium_aligned(16);
    medium_aligned.flip(0);
    medium_aligned.flip(3);
    medium_aligned.flip(10);
    medium_aligned.flip(15);
    assert(medium_aligned.size() == 16);
    assert(!medium_aligned.empty());
    assert(medium_aligned.test(0));
    assert(medium_aligned.test(3));
    assert(medium_aligned.test(10));
    assert(medium_aligned.test(15));
    _UTIL_ASSERT_THROW(medium_aligned.flip(16));
    _UTIL_ASSERT_THROW(medium_aligned.flip(17));


    std::vector<utility::DynamicBitset<8>> bitsets;
    bitsets.resize(256, utility::DynamicBitset<8>(256));
    for (auto i : utility::range(256))
    {
        set_from_int(i, bitsets[i]);

        int64_t v    = 0;
        auto ullongs = bitsets[i].to_ullongs();
        for (auto i : utility::indices(ullongs))
        {
            v += ullongs[i] << i;
        }
        assert(v == i);
    }
    for (auto i : utility::range(255))
    {
        assert(bitsets[i] < bitsets[i + 1]);
    }
    std::set<utility::DynamicBitset<8>> bitsets_set;
    for (auto& b : bitsets)
    {
        bitsets_set.insert(b);
    }
    assert(bitsets.size() == bitsets_set.size());

    utility::DynamicBitset<64> long_bitset(2070);
    utility::DynamicBitset<64> long_bitset_2(2070);
    for (auto i : utility::range(2069))
    {
        long_bitset_2.flip(i);
    }
    assert(long_bitset < long_bitset_2);

    return 0;
}
