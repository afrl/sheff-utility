/// Unit test for DynamicBitset.h

#include "utility/DynamicBitset.h"

#include <assert.h>
#include <iostream>
#include <cinttypes>

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

    return 0; 
}
