/// Unit test for Bitset.h

#include "utility/Bitset.h"

#include <assert.h>
#include <iostream>
#include <cinttypes>


enum class FlagsTest : int8_t
{
    A,
    B,
    _end
};

int main()
{
    utility::bitset<FlagsTest> a = FlagsTest::A;
    assert(a.contains(FlagsTest::A));
    assert(!a.contains(FlagsTest::B));

    utility::bitset<FlagsTest> b = utility::mask(FlagsTest::A) | utility::mask(FlagsTest::B);
    assert(b.contains(FlagsTest::A));
    assert(b.contains(FlagsTest::B));

    return 0; 
}
