/// Unit test for Hash.h

#include "utility/Hash.h"

#include <assert.h>
#include <iostream>

int main()
{
    int int_1 = 0;
    int int_2 = 120;
    std::pair<int, int> pair_1 = { int_1, int_2 };
    assert(utility::hash()(int_1, int_2) == utility::hash()(pair_1));

    assert(utility::hash()(int_1, int_2) != utility::hash()(int_2, int_1));
    assert(utility::hash_sorted()(int_1, int_2) == utility::hash_sorted()(int_2, int_1));

    return 0; 
}
