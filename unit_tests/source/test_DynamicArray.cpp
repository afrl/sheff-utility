/// Unit test for DynamicArray.h

#include "utility/DynamicArray.h"

#include <algorithm>
#include <assert.h>
#include <cinttypes>
#include <iostream>


int main()
{
    utility::DynamicArray<int, int> a = { 1, 2, 3, 4 };
    utility::DynamicArray<int, int> b = { 0, 0, 0, 0 };

    std::copy(a.begin(), a.end(), b.begin());

    for (auto &i : a)
    {
        assert(i==b[&i - &a[0]]);
    }

    std::reverse(b.begin(), b.end());

    int val = 4;
    for (auto &i : b)
    {
        assert(i==val);
        val--;
    }

    val--;

    std::transform(a.begin(), a.end(), a.begin(), [](const int i) { return -1 * i; });

    for (auto &i : a)
    {
        assert(i==val);
        val--;
    }

    return 0;
}