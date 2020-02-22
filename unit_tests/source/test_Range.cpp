/// Unit test for Range.h

#include "utility/Range.h"

#include <array>
#include <assert.h>
#include <iostream>

struct Foo
{
    double a;
};

struct Bar : public Foo
{
    double b;
};

int main()
{
    std::array<Bar, 3> values;
    for (auto& foo_object : utility::range_for_array<Foo>(&values[0], &values[0] + 3))
    {
        foo_object.a = 0.0;
    }
    assert(values[0].a == 0.0);
    assert(values[1].a == 0.0);
    assert(values[2].a == 0.0);

    utility::range_for_array<Foo> range_foo(&values[0], &values[0] + 3);
    auto range_bar = range_foo.as<Bar>();

    return 0; 
}
